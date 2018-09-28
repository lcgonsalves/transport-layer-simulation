# transport-layer-simulation - an implementation of RDT 3.0 as shown in Yanhua Li's CS3615 class #

=============================================================

## Compiling the code: ##
-------------------------
	A makefile has been provided in the root folder. The project can be built with the command
		make
	or
		make all

	If a rebuild is needed, it is recommended to cleanup using
		make clean

## Running the code: ##
	In order to run the code, simply type in terminal:
		./project2

	I have provided sample tests inside the folder 'sampleinputs'. In order to run the code with these sample inputs you can type

		./project2 < sample_input.txt
		./project2 < rand_out_of_order_and_high_loss.txt

	along with other examples inside the folder.

## Current issues ##
--------------------
	The code is currently unable to safely and consistently handle corruption in the layer3. High volume, loss, out of order-all of the other possible errors are consistently and safely handled.


## CHECKSUM ALGORITHM ##
---------------------

This program implements the Fletcher-16 algorithm.

It is important to realize that this algorithm is not perfect, meaning
that some types of corruption may go unnoticed. According to testing, only about 50 out of 100000 corrputions will go unnoticed and reach layer 5. That is an incredibly low (about 0.05%), albeit noticeable fault. Using larger bit checksums could help catch these overflows.
