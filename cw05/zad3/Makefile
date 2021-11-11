CC=gcc
CCFLAGS=-Wall

.PHONY: clean test 5_producers_1_consumer_test

test: 5_producers_1_consumer_test 1_producer_5_consumers_test 5_producers_5_consumers_test
	@:

5_producers_5_consumers_test: consumer.run producer.run
	rm -f test_fifo || true
	mkfifo test_fifo
	@echo 5 producers and 5 consumers test
	@echo Testing for 40-byte chunks
	@./producer.run test_fifo 14 input_files/input_1 40 &
	@./producer.run test_fifo 2 input_files/input_2 40 &
	@./producer.run test_fifo 5 input_files/input_3 40 &
	@./producer.run test_fifo 8 input_files/input_4 40 &
	@./producer.run test_fifo 4 input_files/input_5 40 &
	@./consumer.run test_fifo consumer_out 40 & 
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out
	@killall producer.run || true
	@killall consumer.run || true
	@echo Testing for 9000-byte chunks
	@./producer.run test_fifo 14 input_files/input_1 9000 &
	@./producer.run test_fifo 2 input_files/input_2 9000 &
	@./producer.run test_fifo 5 input_files/input_3 9000 &
	@./producer.run test_fifo 8 input_files/input_4 9000 &
	@./producer.run test_fifo 4 input_files/input_5 9000 &
	@./consumer.run test_fifo consumer_out 9000 & 
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out
	@killall producer.run || true
	@killall consumer.run || true
	rm test_fifo		


1_producer_5_consumers_test: consumer.run producer.run
	rm -f test_fifo || true
	mkfifo test_fifo
	@echo 1 Producer and 5 consumers test.
	@echo Testing for 40-byte chunks
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./consumer.run test_fifo consumer_out 40 &
	@./producer.run test_fifo 14 input_files/input_4 40 
	@./content_test.py input_files/input_4 consumer_out
	@echo Testing for 9000-byte chunks
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./consumer.run test_fifo consumer_out 9000 &
	@./producer.run test_fifo 14 input_files/input_4 9000 
	@./content_test.py input_files/input_4 consumer_out
	@killall producer.run || true
	@killall consumer.run || true

	rm test_fifo

5_producers_1_consumer_test: consumer.run producer.run
	rm -f test_fifo || true
	mkfifo test_fifo
	@echo 5 Producers and 1 consumer test.
	@echo Testing for 40-byte chunks
	@./producer.run test_fifo 14 input_files/input_1 40 &
	@./producer.run test_fifo 2 input_files/input_2 40 &
	@./producer.run test_fifo 5 input_files/input_3 40 &
	@./producer.run test_fifo 8 input_files/input_4 40 &
	@./producer.run test_fifo 4 input_files/input_5 40 &
	@./consumer.run test_fifo consumer_out 40
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out
	@echo Testing for 1000-byte chunks
	@./producer.run test_fifo 14 input_files/input_1 1000 &
	@./producer.run test_fifo 2 input_files/input_2 1000 &
	@./producer.run test_fifo 5 input_files/input_3 1000 &
	@./producer.run test_fifo 8 input_files/input_4 1000 &
	@./producer.run test_fifo 4 input_files/input_5 1000 &
	@./consumer.run test_fifo consumer_out 1000
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out
	@echo "Testing for 7000-byte chunks (>PIPE_BUF)"
	@./producer.run test_fifo 14 input_files/input_1 7000 &
	@./producer.run test_fifo 2 input_files/input_2 7000 &
	@./producer.run test_fifo 5 input_files/input_3 7000 &
	@./producer.run test_fifo 8 input_files/input_4 7000 &
	@./producer.run test_fifo 4 input_files/input_5 7000 &
	@./consumer.run test_fifo consumer_out 7000
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out
	@echo "Testing for 50000-byte chunks (>PIPE_BUF)"
	@./producer.run test_fifo 14 input_files/input_1 7000 &
	@./producer.run test_fifo 2 input_files/input_2 7000 &
	@./producer.run test_fifo 5 input_files/input_3 7000 &
	@./producer.run test_fifo 8 input_files/input_4 7000 &
	@./producer.run test_fifo 4 input_files/input_5 7000 &
	@./consumer.run test_fifo consumer_out 7000
	@./content_test.py input_files/input_1 consumer_out
	@./content_test.py input_files/input_2 consumer_out
	@./content_test.py input_files/input_3 consumer_out
	@./content_test.py input_files/input_4 consumer_out
	@./content_test.py input_files/input_5 consumer_out

	rm test_fifo

consumer.run: consumer.c
	$(CC) $(CCFLAGS) $< -o $@

producer.run: producer.c
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f consumer_out || true
	rm -f test_fifo || true
	rm -f *.run || true
	killall producer.run || true
	killall consumer.run || true
