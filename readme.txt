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
