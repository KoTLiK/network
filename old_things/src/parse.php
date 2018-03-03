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
define("ERR_MSG", "Wrong arguments! Re-run with --help\n");

function help() {
    echo "Help me!";
}

function false_val_required(array $arr, $o) {
    if ($arr[$o] !== false) {
        fwrite(STDERR, "Wrong usage of '--$o'!\n");
        exit(ERR_PARAM);
    }
}

function readInput($input = STDIN) {
    $text = "";
    while (($line = fgets($input)) !== false)
        $text .= $line;
    return $text;
}

$longopts = array(
    "help::", # Bez hodnoty!
    "stats:",
    "loc::", # Bez hodnoty!
    "comments::" # Bez hodnoty!
);
$options = getopt(null, $longopts);

if ($options === false) {
    fwrite(STDERR, ERR_MSG);
    exit(ERR_PARAM);
} else foreach (array_keys($options) as $o)
    if (is_array($options[$o])) {
        # Pri viacnasobnom parametre sa vytvori pole konkretneho parametru
        fwrite(STDERR, ERR_MSG);
        exit(ERR_PARAM);
    }


#   CONFIG   #
$stats = false;
$output = null;
$loc = false;
$comments = false;


foreach (array_keys($options) as $o) {
    switch ($o) {
        case "help":
            false_val_required($options, $o);
            if ((count($options) === 1) && ($argc === count($options)+1)) { help(); exit(IS_OK); }
            fwrite(STDERR, "Argument '--help' has to be used alone!\n");
            exit(ERR_PARAM);
        case "stats":
            $stats = true;
            $output = $options[$o];
            break;
        case "loc":
            false_val_required($options, $o);
            $loc = true;
            break;
        case "comments":
            false_val_required($options, $o);
            $comments = true;
            break;
        default:
            fwrite(STDERR, ERR_MSG);
            exit(ERR_PARAM);
    }
}


if ($argc !== count($options)+1) { # kontrola poctu parametrov
    fwrite(STDERR, ERR_MSG);
    exit(ERR_PARAM);
}

if (!$stats && ($loc || $comments)) {
    fwrite(STDERR, ERR_MSG);
    exit(ERR_PARAM);
}

if ($stats) {
    @$output_file = fopen($output, 'w');
    if (!$output_file) {
        fwrite(STDERR, "Unable to open $output !\n");
        exit(-1); # TODO
    }
}

$input = readInput();

$numOfComments = preg_match_all('/#.*/mu', $input, $matched_comments, PREG_SET_ORDER, 0);
$input = preg_replace('/#.*/mu', '', $input);

preg_match_all('/^(?:\s*)(?<opcode>\S+)(?<args>.*)?$/mu', $input, $matched_instructions, PREG_SET_ORDER, 0);

var_dump($matched_comments);
var_dump($matched_instructions);


//$str = '.IPPcode18
//MOVE <var> <symb>
//CREATEFRAME
//PUSHFRAME #
//POPFRAME
//DEFVAR <var> #asdasdas sdsad
//CALL <label>
//RETURN
//
//PUSHS <symb>
//POPS <var>
//
//# sadas asd a sad  ssss s
//
//ADD <var> <symb1> <symb2>
//SUB <var> <symb1> <symb2>
//MUL <var> <symb1> <symb2>
//IDIV <var> <symb1> <symb2>
//LT <var> <symb1> <symb2>#sad sdsdsd
//GT <var> <symb1> <symb2>
//EQ <var> <symb1> <symb2>
//AND <var> <symb1> <symb2> # sad sda dsa
//OR <var> <symb1> <symb2>
//NOT <var> <symb1> <symb2>
//INT2CHAR <var> <symb>
//STRI2INT <var> <symb1> <symb2>
//
//READ <var> <type>
//WRITE <symb>
//
//CONCAT <var> <symb1> <symb2>
//STRLEN <var> <symb>
//GETCHAR <var> <symb1> <symb2>
//SETCHAR <var> <symb1> <symb2>
//
//TYPE <var> <symb>
//
//LABEL <label>
//JUMP <label>
//JUMPIFEQ <label> <symb1> <symb2>
//JUMPIFNEQ <label> <symb1> <symb2>
//
//DPRINT <symb>
//BREAK
//';
