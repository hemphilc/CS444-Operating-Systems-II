echo Testing sstf-iosched...

TEST_1=test1.txt
TEST_2=test2.txt

LEN=100

for i in `seq 1 $LEN`
do
        echo "TEST1 I/O $i" >> $TEST_1
        echo "TEST2 I/O $i" >> $TEST_2
        echo "TEST1 I/O AGAIN $i" >> $TEST_1
        echo "TEST2 I/O AGAIN $i" >> $TEST_2
done

TEST_SIZE1=$(stat -c%s "$TEST_1")
TEST_SIZE2=$(stat -c%s "$TEST_2")

echo TEST_SIZE1
echo TEST_SIZE2
