
.PHONY: all
all:
	./analyze_fitts.py -a simple_time -d patrick > aggregate.txt
	./analyze_fitts.py -a simple_time -d molly >> aggregate.txt
	./analyze_fitts.py -a simple_time -d john >> aggregate.txt
	./analyze_fitts.py -a simple_time -d nick >> aggregate.txt
	./analyze_fitts.py -a simple_time -d kevin >> aggregate.txt

clean:
	rm aggregate.txt
