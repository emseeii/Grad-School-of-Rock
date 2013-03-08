OUTPUT = distort
DISTORT = distort
FREQ = freqDetector
EXT = extender

#install		: $(TARGET)
#		sudo cp $(TARGET) /usr/local/bin/rra$(TARGET)

$(OUTPUT)	: $(DISTORT).c $(FREQ).c $(EXT).c
		gcc -o $(OUTPUT) -Wall -g -L../../lib $(DISTORT).c $(FREQ).c $(EXT).c -L../../lib -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET)
