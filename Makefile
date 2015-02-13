all:
	gcc WEH16002-spitest.c -o WEH16002-spitest -lbcm2835

clean:
	rm WEH16002-spitest
