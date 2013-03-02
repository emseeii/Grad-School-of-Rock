TARGET = distort

#install		: $(TARGET)
#		sudo cp $(TARGET) /usr/local/bin/rra$(TARGET)

$(TARGET)	: $(TARGET).c
		gcc -o $(TARGET) -Wall -g -L../../lib $(TARGET).c -L../../lib -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET)
