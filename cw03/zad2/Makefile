

.PHONY: clean runtests

clean:
	cd cw01-zrodlo-oryginalne && $(MAKE) clean
	cd cw01-zrodlo-procesy && $(MAKE) clean

runtests:
	cd cw01-zrodlo-procesy && $(MAKE) merger-test-static.run && echo exit | ./merger-test-static.run > test_result.txt
	mv cw01-zrodlo-procesy/test_result.txt ./threaded_result.txt
	cd cw01-zrodlo-oryginalne && $(MAKE) merger-test-static.run && echo exit | ./merger-test-static.run > test_result.txt
	mv cw01-zrodlo-oryginalne/test_result.txt ./nonthreaded_result.txt