default: all
.PHONY: all

check:
	./analyze_fitts.py -a check -d jake
	./analyze_fitts.py -a check -d john
	./analyze_fitts.py -a check -d katie
	./analyze_fitts.py -a check -d kevin
	./analyze_fitts.py -a check -d molly
	./analyze_fitts.py -a check -d nader
	./analyze_fitts.py -a check -d nelson
	./analyze_fitts.py -a check -d nick
	./analyze_fitts.py -a check -d patrick
	./analyze_fitts.py -a check -d warren

all:
	./analyze_fitts.py -a simple_time -d jake     > aggregate.txt
	./analyze_fitts.py -a simple_time -d john     >> aggregate.txt
	./analyze_fitts.py -a simple_time -d katie    >> aggregate.txt
	./analyze_fitts.py -a simple_time -d kevin    >> aggregate.txt
	./analyze_fitts.py -a simple_time -d molly    >> aggregate.txt
	./analyze_fitts.py -a simple_time -d nader    >> aggregate.txt
	./analyze_fitts.py -a simple_time -d nelson   >> aggregate.txt
	./analyze_fitts.py -a simple_time -d nick     >> aggregate.txt
	./analyze_fitts.py -a simple_time -d patrick  >> aggregate.txt
	./analyze_fitts.py -a simple_time -d warren   >> aggregate.txt

clean:
	rm aggregate.txt
