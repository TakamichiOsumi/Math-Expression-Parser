# Math-Expression-Parser

Building submodule for my other repository.

Parse math expressions that contain application-defined variables.

## `libmexpr` library functions

| Function | Description |
| ---- | ---- |
| start_ineq_mathexpr_parse | Parse math expression that contains inequality operators |
| start_logical_mathexpr_parse | Parse math expression that contains logical operators |
| start_mathexpr_parse | Parse arithmetic expression |

## How to build and test

```console
$ git clone https://github.com/TakamichiOsumi/Math-Expression-Parser.git
$ cd Math-Expression-Parser
$ git submodule init
$ git submodule update
$ make
$ make test
```
