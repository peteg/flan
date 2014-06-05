# FLAN: a functional language

FLAN is a toy functional language realised as an interpreter written
in C. It was intended to be a vehicle for assignments for first year
students at CSE, UNSW circa 2008. It supports higher-order functions,
simple types, user-defined algebraic datatypes, and strict and lazy
evaluators. There is also a simpler first-order strict
evaluator. There is no type inference or parametric polymorphism.

John Shepherd <http://www.cse.unsw.edu.au/~jas/> commissioned this
project and Peter Gammie <http://peteg.org/> executed it. It owes some
inspiration to C/Java compiler first-year assignments dating back to
1996. Andrew Taylor <http://www.cse.unsw.edu.au/~andrewt/> was
responsible for some of these.

FLAN comes with FLANGE - the FLAN graphics engine, which is based on
the functional geometry ideas originated by Peter Henderson. The
system renders such pictures as SVG files.

Additionally there are many small tests that contain some classical
functional programming techniques. Some were scraped from the GHC test
suite, others from textbooks (Bird/Wadler, Thompson, ...) and
functional pearls. Provenance and authorship should be clear for
non-trivial examples; if it is not, please tell me.

## Building

Dependencies:

* A modern C compiler supporting at least C99. FLAN was developed
  using gcc-4.2 under Mac OS X 10.6 (and earlier), and some archaisms
  remain.

* perl, to generate the header file for the lexer.

* flex, to generate the lexer itself.

* H.-J. Boehm's garbage collector (optional)

Building it should simply be a matter of typing "make". Investigate
the Makefile if it is not.

The code was intended to be warning-free (modulo the output of flex),
and is with GCC but not clang.

## Execution

The command-line arguments for the flan executable should be
self-explanatory:

$ flan --help

## Examples

Suffix "-fail" means the test should be flagged as an error.

0 - basic syntax
1 - simple functions over Int, tuples
2 - simple functions over lists
3 - types, type errors
4 - higher-order functions (that work with strict eval)
5 - scoping (turn the type checker off)
6 - serious list algorithms
7 - graphs
8 - long programs (that work with strict eval)
9 - laziness, user-defined types, ...

## Ideas for further development

### FLAN

* Accurate garbage collection.

* REPL

* Types: parametric polymorphism

* More built-in functions: trigonometry, ...

### FLANGE

* [L-systems](http://en.wikipedia.org/wiki/L-system), probably eased
  by implementing turtle graphics first.
 * Koch curve
 * Dragon curve

* Compile FLANGE images to SVG and JavaScript, allowing zooming of
  (co-)recursive images and animations (= lazy list of FLANGE images).

* Colours.

* Text.

## Bugs and infelicities

Possibly quite stale.

* Parser: verify we've hit EOF when parsing is finished.

* Scope within a 'let' block is a bit strange:
 * constants can use earlier definitions in the same block.
 * functions can use any definition in the same block.

* Fix the scoping up in the higher-order strict evaluator.
 * decide on let or letrec and make it stick.
 * Ensure that the same variable is not given several definitions in
   the same scope.

* Reduce duplication between e_expr and e_binary_op

* Write a test that establishes that updating closures is not correct
  (eval-hol:236).
 * I think one can do this by using a HOF twice.
 * Check very carefully about updating values of all kinds.

* Type checker should take evaluator scope into account.
 * e.g. constants are not in-scope in their own definitions in strict
   evaluators.
