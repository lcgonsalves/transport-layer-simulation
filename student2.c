#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "project2.h"
#define TIMER_INCREMENT 8

// globals
int ASeqNum = 0;
int BSeqNum = 0;
int AWaitingAck = FALSE;
struct pkt A_recent_packet;
struct pkt B_recent_packet;
extern int TraceLevel;

// buffers
struct pkt A_packet_buf[20];
 
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
 * Does the checksum
 */
int checksum(char* target)
{
	unsigned long sum = 0;
	// add up all characters to a long
	for (int i = 0; i < MESSAGE_LENGTH; i++)
	{
		sum += target[i];
	}

	// count up overflow bits
	//		-> ...111111100000000 <- 8 bits are zero so we count only overflow
	unsigned long bit_mask = (~0) << (sizeof(char) * 8);
	unsigned long overflow = (sum & bit_mask);
	unsigned int count = 0;
	while (overflow)
	{
		count += overflow & 1; 	// add if last bit is 1
		overflow >>= 1;			// bit shift right until we reach 0
	}

	// add overflow bits to final sum
	sum += count;

	// use inverted mask to "remove" overflow bits
	sum = (sum & ~bit_mask);

	// ret final count
	return sum;
}

/**
 * Does a checksum and compares it to the given checksum
 * of the other packet.
 */
int isCorrupt(struct pkt recv_packet)
{
	int csum 		= recv_packet.checksum;
	int new_csum	= checksum(recv_packet.payload);
	if (TraceLevel > 0)
		printf("Given checksum: 0x%x // New checksum 0x%x\n", csum, new_csum);

	int result = (~csum & new_csum); // bitwise & will be zero if they're the same

	if (TraceLevel > 0)
		printf("Result of comparison is %i\n", result);

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
	// only do anything if we're not waiting for ack. buffer incoming message
	if (AWaitingAck) return;
	// make packet
	struct pkt packet;
	memset(&packet, 0, sizeof(packet));
	packet.acknum 	= -1; // no need for ack because this is the sender side
	packet.checksum = checksum(message.data); //todo: implement checksum
	packet.seqnum 	= ASeqNum;
	memcpy(packet.payload, message.data, MESSAGE_LENGTH);

	// send to layer 3, save copy if it gets lost
	A_recent_packet = packet;
	tolayer3(AEntity, packet);
	AWaitingAck = TRUE;

	// start timer
	startTimer(AEntity, TIMER_INCREMENT);
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
	if (TraceLevel > 0)
		printf("A: Received packet: %s with ACKNUM %i. Was expecting %i\n", packet.payload, packet.acknum, ASeqNum);

	// check for corruption and order
	if (!isCorrupt(packet) && packet.acknum == ASeqNum)
	{
		// stop timer, advance seqnum
		stopTimer(AEntity);
		ASeqNum = !ASeqNum; // since there's only 2 options, either 0 or !0
		if (TraceLevel > 0)
		{
			printf("A: Packet %s ACK %i received.\nSequence num is now %i\n",
				packet.payload,
				packet.acknum,
				ASeqNum);
		}

		// now we can send a new packet
		AWaitingAck = FALSE;
	}
	// if packet is either corrupt or out of order, do nothing
	else if (TraceLevel > 0)
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
	if (TraceLevel > 0) printf("Timeout. Resending packet %s\n", A_recent_packet.payload);
	tolayer3(AEntity, A_recent_packet);

	// reset timer
	startTimer(AEntity, TIMER_INCREMENT);
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
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
	if (TraceLevel > 0)
			printf("B: Received packet: %s with ACKNUM %i. Was expecting %i\n", packet.payload, packet.seqnum, ASeqNum);

	// check for corruption
	if (!isCorrupt(packet) && packet.seqnum == BSeqNum)
	{	// if not currupt and in order
		// extract data
		struct msg received_msg;
		memcpy(received_msg.data, packet.payload, MESSAGE_LENGTH);

		// deliver data
		tolayer5(BEntity, received_msg);

		// send ACK
		packet.acknum = BSeqNum;
		tolayer3(BEntity, packet);
		B_recent_packet = packet;

		// change seqnum
		BSeqNum = !BSeqNum;
		if (TraceLevel > 0) printf("B: Packet %s is uncorrupted and in order.\nSequence num is now %i\n", packet.payload, ASeqNum);

	}
	else {
		if (TraceLevel > 0){
			printf("B: Packet %s is out of order or corrupted. Waiting for correct ACK or timeout.\n",
									packet.payload);
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
}

