#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "project2.h"
#include "./linkedlist/linkedlist.h"
#define TIMER_INCREMENT 1800

// globals
int ASeqNum = 0;
int BSeqNum = 0;
int AWaitingAck = FALSE;
struct pkt A_recent_packet;
struct pkt B_recent_packet;
linkedlist* A_msg_queue; // queue
extern int TraceLevel;
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

// ***************** ADDITIONAL HELPERS ******************************/

/**
 * Creates a Fletchet-16bit checksum of the entire packet minus the checksum field
 * (which is reset to 0 for the purpose of this algorithm)
 *
 * Note: this is a modified implementation of Fletcher16 checksum
 * found on https://en.wikipedia.org/wiki/Fletcher%27s_checksum#Check_bytes
 *
 * It is important to realize that this algorithm is not perfect, meaning
 * that some types of corruption may go unnoticed. According to testing, only about
 * 50 out of 100000 corrputions will go unnoticed and reach layer 5. That is an
 * incredibly low (about 0.05%), albeit noticeable fault. Using larger bit checksums
 * could help catch these overflows.
 */
int checksum(struct pkt packet) // no ref or pointer, so when called a copy of the packet will be made
{
	// set checksum of copy packet to zero so no interference with original checksum
	packet.checksum = 0;
	size_t pkt_size = sizeof(struct pkt);

	// cast packet to an array of bytes
	char* byte_sequence = (char*) &packet;
	uint16_t sum1 = 0; // could use uint_16 for consistency across platforms
	uint16_t sum2 = 0;

	int i;
	// add up all bytes in struct
	for (i = 0; i < pkt_size; i++)
	{
		sum1 = (sum1 + byte_sequence[i]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	return (sum2 << 8) | sum1;
}

/**
 * Does a checksum and compares it to the given checksum
 * of the other packet.
 */
int isCorrupt(struct pkt recv_packet)
{
	int csum 		= recv_packet.checksum;
	int new_csum	= checksum(recv_packet);
	if (TraceLevel > 2)
		printf("		Given checksum: 0x%x // New checksum 0x%x\n", csum, new_csum);

	int result = csum != new_csum; // bitwise & will be zero if they're the same, can do != as well

	if (TraceLevel > 2)
		printf("		Result of comparison is %i\n", result);

	return result;
}


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
	// make packet
	struct pkt packet;
	memset(&packet, 0, sizeof(packet));
	packet.acknum 	= ASeqNum;
	packet.seqnum 	= ASeqNum;
	memcpy(packet.payload, message.data, MESSAGE_LENGTH);

	packet.checksum = checksum(packet); // create checksum

	// only do anything if we're not waiting for ack. buffer incoming message
	if (AWaitingAck)
	{
		if (TraceLevel > 2) printf("A: Currently awaiting response, message %s buffered.\n", message.data);
		struct msg *buffer = (struct msg*) malloc (sizeof(struct msg));
		memcpy(buffer, &message, sizeof(struct msg));
		insert_end(A_msg_queue, buffer);
		return; // so packet is not send immediately
	}

	// send to layer 3, save copy if it gets lost
	A_recent_packet = packet;
	tolayer3(AEntity, packet);
	AWaitingAck = TRUE;

	// start timer
	startTimer(AEntity, TIMER_INCREMENT);
	if (TraceLevel > 2)
		printf("A: Timer started, %s sent with SEQ: %i.\n", packet.payload, packet.seqnum);
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
	// not needed - only doing unidirectional
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {
	if (TraceLevel > 2)
		printf("A: Received packet: %s with ACKNUM %i. Was expecting %i\n", packet.payload, packet.acknum, ASeqNum);

	// check for corruption and order
	if (!isCorrupt(packet) && packet.acknum == ASeqNum)
	{
		// stop timer, advance seqnum
		stopTimer(AEntity);
		ASeqNum = !ASeqNum; // since there's only 2 options, either 0 or !0
		if (TraceLevel > 2)
		{
			printf("A: Packet %s ACK %i successfully received.\nA: Sequence num is now %i\n",
				packet.payload,
				packet.acknum,
				ASeqNum);
		}

		// now we can send a new packet. check queue
		AWaitingAck = FALSE;
		struct msg *queued_msg = pop(A_msg_queue);
		if (queued_msg)
		{	// if we have a packet waiting in queue,call output on it
			A_output(*queued_msg); // pointer dereference = send copy of packet
			free(queued_msg);
		}
	}
	// if packet is either corrupt or out of order, do nothing
	else if (TraceLevel > 2)
			printf("A: Packet %s is out of order or corrupted. Waiting for correct ACK or timeout.\n",
					packet.payload);
	// after this, wait for layer 5 call
}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	// resend packet
	if (TraceLevel > 2) printf("A: Timeout. Resending packet %s\n", A_recent_packet.payload);
	tolayer3(AEntity, A_recent_packet);

	// reset timer
	startTimer(AEntity, TIMER_INCREMENT);
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	// initialize linked list
	A_msg_queue = init_linkedlist();
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	if (TraceLevel > 2)
	{
		printf("B: Received packet: %s with SEQNUM %i. Was expecting %i\n",
							packet.payload,
							packet.seqnum,
							BSeqNum);
	}

	// check for corruption
	if (!isCorrupt(packet) && packet.seqnum == BSeqNum)
	{	// if not currupt and in order
		// extract data
		struct msg received_msg;
		memcpy(received_msg.data, packet.payload, MESSAGE_LENGTH);

		// deliver data
		tolayer5(BEntity, received_msg);

		// send ACK
		tolayer3(BEntity, packet);
		memcpy(&B_recent_packet, &packet, sizeof(struct pkt));

		// change seqnum
		BSeqNum = !BSeqNum;
		if (TraceLevel > 2)
		{
			printf("B: Packet %s is uncorrupted and in order.\nB: Sequence num is now %i\n",
				packet.payload,
				BSeqNum);
		}

	}
	else {
		if (TraceLevel > 2){
			if (packet.checksum == B_recent_packet.checksum || packet.seqnum == B_recent_packet.seqnum){
				printf("B: Received recent packet (%s). ACK was corrupted. Re-sending recent packet with ACK: %i.\n",
													packet.payload,
													B_recent_packet.acknum);
			}
			else
				printf("B: Packet %s is out of order or corrupted. Sending NAK (ack with value: %i).\n",
										packet.payload,
										B_recent_packet.acknum);
		}

		// resend ack for old packet
		tolayer3(BEntity, B_recent_packet);
	}

}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	// set recent packet to something with ack opposite to starting state
	struct pkt temp;
	temp.acknum = !BSeqNum;
	strcpy(temp.payload, "INIT");
	temp.seqnum = !BSeqNum;
	temp.checksum = checksum(temp);

	B_recent_packet = temp;
}

