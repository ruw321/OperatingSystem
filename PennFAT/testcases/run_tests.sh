apt-get install expect
chmod a+x ../pennfat

../pennfat <<< 'mkfs move_copy 1 0'
echo "running move testcase"
expect ./move/move.exp
sleep 2
echo "running copy testcase"
expect ./copy/copy.exp
sleep 2
rm move_copy

../pennfat <<< 'mkfs fill_clear 1 0'
echo "running fill testcase"
./fill/fill.exp
sleep 10
echo "running clear testcase"
./clear/clear.exp
sleep 10
rm fill_clear
