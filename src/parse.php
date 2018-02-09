<?php
/**
 * Created by IntelliJ IDEA.
 * User: milan
 * Date: 9.2.2018
 * Time: 21:04
 */

iconv_set_encoding("internal_encoding", "UTF-8");

define("IS_OK", 0);
define("ERR_PARAM", 10);

function help() {
    echo "Help me!";
}

function false_val_required(array $arr, $o) {
    if ($arr[$o] !== false) {
        fwrite(STDERR, "Nespravne pouzitie parametru '--$o'!\n");
        exit(ERR_PARAM);
    }
}

$longopts = array(
    "help::", # Bez hodnoty!
    "stats::",
    "loc::",
    "comments::"
);
$options = getopt(NULL, $longopts);




$re = '/#.*/mu';
$str = '.IPPcode18
MOVE <var> <symb>
CREATEFRAME
PUSHFRAME #
POPFRAME 
DEFVAR <var> #asdasdas sdsad 
CALL <label>
RETURN

PUSHS <symb>
POPS <var>

# sadas asd a sad  ssss s

ADD <var> <symb1> <symb2>
SUB <var> <symb1> <symb2>
MUL <var> <symb1> <symb2>
IDIV <var> <symb1> <symb2>
LT <var> <symb1> <symb2>#sad sdsdsd 
GT <var> <symb1> <symb2>
EQ <var> <symb1> <symb2>
AND <var> <symb1> <symb2> # sad sda dsa
OR <var> <symb1> <symb2>
NOT <var> <symb1> <symb2>
INT2CHAR <var> <symb>
STRI2INT <var> <symb1> <symb2>

READ <var> <type>
WRITE <symb>

CONCAT <var> <symb1> <symb2>
STRLEN <var> <symb>
GETCHAR <var> <symb1> <symb2>
SETCHAR <var> <symb1> <symb2>

TYPE <var> <symb>

LABEL <label>
JUMP <label>
JUMPIFEQ <label> <symb1> <symb2>
JUMPIFNEQ <label> <symb1> <symb2>

DPRINT <symb>
BREAK
';

preg_match_all($re, $str, $matches, PREG_SET_ORDER, 0);

// Print the entire match result
var_dump($matches);


$re = '/^(?:\s*)(?<opcode>\S+)(?<args>.*)?$/mu';

preg_match_all($re, $str, $matches, PREG_SET_ORDER, 0);

// Print the entire match result
var_dump($matches);
