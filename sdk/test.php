<?php
/**
 * Created by PhpStorm.
 * User: kevin
 * Date: 14-4-10
 * Time: 下午1:07
 */

define('OUTPUT',intval($argv[1]));
$faild = 0;
$totalCost = 0;
function test($str) {
    global $faild;
    global $totalCost ;
	global $argv;
    $len = strlen($str) + 12;
    $len = str_pad($len, 10, "0", STR_PAD_LEFT);
	if($argv[1] == 2) {
		$str = $len ."01". $str;
	} else {
		$str = $len ."00". $str;
	}

//	echo $str;
    //$fd = fsockopen('192.168.137.128', 8615);
    $fd = fsockopen('127.0.0.1', 8615);
    $start = microtime(1);
    fwrite($fd, $str, strlen($str));
    $s = '';
    while ($r = fgets($fd, 1024)) {
        $s .= $r;
    }
    fclose($fd);
    $cost = microtime(1) - $start;

    if(empty($s)) {
        $faild++;
    }
    $totalCost+= $cost;
    $code = substr($s, 0, 3);
    $err = array(
        '097' => '线程太多，服务端太忙',
        '098' => '内容不能为空',
        '200' => '包含关键字',
        '100' => '没有包含关键字',
    );

    $error = $err[$code];

    if(OUTPUT)
    echo '返回状态：' . $error . "\n";
    if ($code == 200) {
        $s = substr($s, 3);
        $list = explode("\n", $s);
        array_pop($list);//去掉最后一\n
        foreach ($list as $k => $v) {
            list($start,$len,$replace) = explode(' ',$v);
            $list[$k] = '起始字符:'.$start.' '.substr($str,$start,$len);
			if($replace) {
				for($i=$start;$i< $start + $len;$i++) {
					$str[$i] = '*';
				}
			}
        }
        $list['cost'] = $cost;
        $list['after'] = $str;
        if(OUTPUT)
        print_r($list);
    }
}
if(OUTPUT) {
    test($argv[2]);
 //  test("日 日 毛泽东 av 日日共产党 xxx CC小雪M<(*&*&%&^%$%^$@#$%^)_+_+\"3424");die;
	die;
   }
//只匹配1个字
test("泽东");
//连续匹配
test("共产党毛泽毛泽东");
//某个匹配在中间匹配
test("日毛泽东 共产党");
test("共产党日毛泽东");
//某2个匹配在中间匹配
test("日日毛泽东");
//空格
test("日 毛泽东");
test("日日 毛泽东");
test("日 日 毛泽东");

test("日 日 毛泽东 日日共产党");
test("日 日 毛泽东 av 日日共产党 xxx CC小雪M<(*&*&%&^%$%^$@#$%^)_+_+\"3424");
//特殊符号
for($i=0;$i<10000;$i++) {
    test("日 日 毛泽东 av 日日共产党 xxx CC小雪M<(*&*&%&^%$%^$@#$%^)_+_+\"3424");
}

echo "失败:".$faild.' 耗时'.$totalCost."\n";


