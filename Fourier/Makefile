
OB += \
./FourierTrans.o 
	
%.o:
	g++  -O0 -g3 -Wall -c  FourierTrans.cpp  `pkg-config --cflags --libs opencv`

all:$(OB)
	@echo 'Building target: $@'
	g++  -o "FourierTrans" $(OB)  `pkg-config --cflags --libs opencv`
	@echo 'Finished building target: $@'


clean:
	-rm $(OB) FourierTrans

