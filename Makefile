
all: waf_detector

waf_detector: waf_detector.cpp
	g++ -o waf_detector waf_detector.cpp -lcurl

clean:
	rm -f waf_detector






