echo Testing sstf-iosched...

dmesg --clear

rm test1.txt
rm test2.txt
rm sstf-iosched_test_output.txt

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

cat test1.txt
cat test2.txt

touch sstf-iosched_test_output.txt

dmesg >> sstf-iosched_test_output.txt

cat sstf-iosched_test_output.txt
