# Math-Expression-Parser

Building submodule for my other repository.

Parse math expressions that contain application-defined variables.

## `libmexpr` library functions

| Function | Description |
| ---- | ---- |
| start_ineq_mathexpr_parse | Parse math expression that contains inequality operators |
| start_logical_mathexpr_parse | Parse math expression that contains logical operators |
| start_mathexpr_parse | Parse arithmetic expression |

All of those functions return true if those parse processings are successful. Otherwise, return false. Users who want to parse string that may match any of them should call `start_mathexpr_parse`, `start_ineq_mathexpr_parse` and `start_logical_mathexpr_parse` in order. When one of the consecutive function calls returns true, it menas the parsed string is categorized into the corresponding math expression.

## How to build and test

```console
$ git clone https://github.com/TakamichiOsumi/Math-Expression-Parser.git
$ cd Math-Expression-Parser
$ git submodule init
$ git submodule update
$ make
$ make test
```
