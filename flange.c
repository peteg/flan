/*
 * FLANGE, the FLAN graphics engine.
 *
 * Based on: Peter Henderson, "Functional Geometry", Symposium on LISP
 * and Functional Programming, 1982.
 * http://doi.acm.org/10.1145/800068.802148
 *
 * A more recent account: Peter Henderson, "Functional Geometry",
 * Higher-Order and Symbolic Computation 15(4): 349-365 (2002).
 * http://dx.doi.org/10.1023/A:1022986521797
 *
 * Also: P. R. Eggert and K. P. Chow, "Logic programming graphics and
 * infinite terms", Technical Report, Department of Computer Science,
 * University of California, Santa Barbara, June 1983.
 *
 * Henderson's work implemented in Lisp, from which I scraped the
 * concrete pictures:
 *
 *    http://www.frank-buss.de/lisp/functional.html
 *
 * There is also some information in Abelson and Sussman's "Structure
 * and Interpretation of Computer Programs".
 *
 * Peter Gammie, peteg42@gmail.com
 * Commenced August 2008.
 *
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "environment.h"
#include "errors.h"
#include "eval.h"
#include "flange.h"
#include "graphics.h"
#include "lexer.h"
#include "parser.h"
#include "symbols.h"
#include "types.h"

/****************************************/
/* Renderer state. */

typedef struct {
  graphics_t *g;

  /* Henderson's three vectors. */
  vector_t origin; /* a: Lower-left corner of the box. */
  vector_t bottom; /* b: Bottom of the picture. */
  vector_t left;   /* c: Left edge of the picture. */
} render_t;

/****************************************/
/* Vector arithmetic over vector_t. */

static vector_t vec_0 = {0, 0};

static vector_t vec_add(vector_t a, vector_t b)
{
  vector_t r = { a.x + b.x
               , a.y + b.y};

  return r;
}

static vector_t vec_sub(vector_t a, vector_t b)
{
  vector_t r = { a.x - b.x
               , a.y - b.y};

  return r;
}

static vector_t vec_scale(vector_t a, double c)
{
  vector_t r = { a.x * c
               , a.y * c};

  return r;
}

static double vec_abs_cross_product(vector_t a, vector_t b)
{
  double result = a.x * b.y - a.y * b.x;

  return result < 0 ? -result : result;
}

static double vec_length(vector_t a)
{
  return sqrt(a.x * a.x + a.y * a.y);
}

/****************************************/
/* FLANGE drawing and geometry types and functions.
 *
 * Extend the standard builtin functions and types with
 * FLANGE-specific ones. Use the parser so we don't have to build the
 * data structures tediously by hand.
 */
static char *flangeTypes =
  "data Shape = Circle (Num, Num) Num            \
              | Line (Num, Num) (Num, Num)       \
   data Picture                                  \
              = Above Num Num Picture Picture    \
              | Beside Num Num Picture Picture   \
              | Canvas Num Num [Shape]           \
              | Empty                            \
              | Flip Picture                     \
              | Overlay Picture Picture          \
              | Rotate Picture";

static list_t *flange_types(void)
{
  list_t *types;

  lex_string_start(flangeTypes);
  lex();
  types = typedefs();
  lex_string_end();

  return types;
}

static env_t *flange_bindings(void)
{
  return builtin_bindings();
}

/****************************************/
/* Render Canvas shapes. */

typedef struct {
  render_t *r;
  double width, height;
} canvas_closure_t;

/* FIXME verify all the circle stuff. */
static void render_circle(canvas_closure_t *c, vector_t centre, double radius)
{
  render_t *r = c->r;

  graphics_ellipse(r->g,
                   vec_add(vec_add(r->origin, vec_scale(r->bottom, centre.x / c->width)),
                           vec_scale(r->left, centre.y / c->height)),
                   vec_length(vec_scale(r->bottom, radius / c->width)),
                   vec_length(vec_scale(r->left,   radius / c->height)),
                   r->bottom);
}

static void render_line(canvas_closure_t *c, vector_t j, vector_t k)
{
  render_t *r = c->r;

  graphics_line(r->g,
                vec_add(vec_add(r->origin, vec_scale(r->bottom, j.x / c->width)),
                        vec_scale(r->left, j.y / c->height)),
                vec_add(vec_add(r->origin, vec_scale(r->bottom, k.x / c->width)),
                        vec_scale(r->left, k.y / c->height)));
}

static int render_shape(canvas_closure_t *c, value_t *shape)
{
  shape = thunk_force(shape);

  switch(shape->type) {
  default:
  case v_unused:
    if(*(int *)null_ptr) {
      printf("should crash.\n");
    }
    break;

  case v_datacons:
    if(strcmp(datacons_tag(shape), "Circle") == 0) {
      value_t *centre = thunk_force(list_nth(datacons_params(shape), 0));
      double radius   = num_val(thunk_force((value_t *)list_nth(datacons_params(shape), 1)));
      vector_t vec_c;

      vec_c.x = num_val(thunk_force((value_t *)list_nth(tuple_val(centre), 0)));
      vec_c.y = num_val(thunk_force((value_t *)list_nth(tuple_val(centre), 1)));

      render_circle(c, vec_c, radius);
    } else if(strcmp(datacons_tag(shape), "Line") == 0) {
      value_t *start = thunk_force(list_nth(datacons_params(shape), 0));
      value_t *end   = thunk_force(list_nth(datacons_params(shape), 1));
      vector_t a, b;

      a.x = num_val(thunk_force((value_t *)list_nth(tuple_val(start), 0)));
      a.y = num_val(thunk_force((value_t *)list_nth(tuple_val(start), 1)));
      b.x = num_val(thunk_force((value_t *)list_nth(tuple_val(end), 0)));
      b.y = num_val(thunk_force((value_t *)list_nth(tuple_val(end), 1)));

      render_line(c, a, b);
    } else {
      printf("render_shape: value is not a Line.\n");
      print_value(stdout, shape);
      error("");
    }
    break;
  }

  return 1;
}

/****************************************/
/* The recursive renderer. */

static void f_render(render_t *r, value_t *v);

static void f_above(render_t *r, value_t *v)
{
  list_t *params = datacons_params(v);
  double top  = num_val(thunk_force((value_t *)list_nth(params, 0)));
  double bot = num_val(thunk_force((value_t *)list_nth(params, 1)));
  value_t *topPic = (value_t *)list_nth(params, 2);
  value_t *botPic = (value_t *)list_nth(params, 3);
  render_t newR;
  double split = bot / (bot + top);

  newR = *r;
  newR.origin = vec_add(r->origin, vec_scale(r->left, split));
  newR.left = vec_scale(r->left, top / (bot + top));
  f_render(&newR, topPic);

  newR = *r;
  newR.left = vec_scale(r->left, bot / (bot + top));
  f_render(&newR, botPic);
}

static void f_beside(render_t *r, value_t *v)
{
  list_t *params = datacons_params(v);
  double left  = num_val(thunk_force((value_t *)list_nth(params, 0)));
  double right = num_val(thunk_force((value_t *)list_nth(params, 1)));
  value_t *leftPic  = (value_t *)list_nth(params, 2);
  value_t *rightPic = (value_t *)list_nth(params, 3);
  render_t newR;
  double split = left / (left + right);

  newR = *r;
  newR.bottom = vec_scale(r->bottom, split);
  f_render(&newR, leftPic);

  newR = *r;
  newR.origin = vec_add(r->origin, vec_scale(r->bottom, split));
  newR.bottom = vec_scale(r->bottom, right / (left + right));
  f_render(&newR, rightPic);
}

static void f_canvas(render_t *r, value_t *v)
{
  list_t *params = datacons_params(v);
  double width  = num_val(thunk_force((value_t *)list_nth(params, 0)));
  double height = num_val(thunk_force((value_t *)list_nth(params, 1)));
  value_t *shapes = thunk_force((value_t *)list_nth(params, 2));
  canvas_closure_t c;

  c.r = r;
  c.width = width;
  c.height = height;

  while(shapes->type == v_datacons
        && strcmp(datacons_tag(shapes), listConsTag) == 0) {
    render_shape(&c, list_nth(datacons_params(shapes), 0));
    shapes = thunk_force(list_nth(datacons_params(shapes), 1));
  }
}

static void f_empty(render_t *r, value_t *v)
{
  UNUSED(r);
  UNUSED(v);
}

static void f_flip(render_t *r, value_t *v)
{
  render_t newR = *r;

  newR.origin = vec_add(newR.origin, newR.bottom);
  newR.bottom = vec_sub(vec_0, newR.bottom);

  f_render(&newR, ((value_t *)list_nth(datacons_params(v), 0)));
}

static void f_overlay(render_t *r, value_t *v)
{
  list_t *params = datacons_params(v);
  value_t *pic1 = (value_t *)list_nth(params, 0);
  value_t *pic2 = (value_t *)list_nth(params, 1);

  f_render(r, pic1);
  f_render(r, pic2);
}

static void f_rotate(render_t *r, value_t *v)
{
  render_t newR = *r;

  newR.origin = vec_add(r->origin, r->bottom);
  newR.bottom = r->left;
  newR.left = vec_sub(vec_0, r->bottom);

  f_render(&newR, ((value_t *)list_nth(datacons_params(v), 0)));
}

static void f_render(render_t *r, value_t *v)
{
  /* For infinite pictures: if the area is less than some constant, stop. */
  if(vec_abs_cross_product(r->bottom, r->left) < 100) {
    return;
  }

  v = thunk_force(v);

  switch(v->type) {
  default:
  case v_unused:
    error("flange: Expected 'main' to be bound to an expression of type 'Picture'.\n");
    break;

  case v_datacons:
    /* IMPROVE: Not very efficient: use the symbol table and function pointers? */
    if(strcmp(datacons_tag(v), "Above") == 0) {
      f_above(r, v);
    } else if(strcmp(datacons_tag(v), "Beside") == 0) {
      f_beside(r, v);
    } else if(strcmp(datacons_tag(v), "Canvas") == 0) {
      f_canvas(r, v);
    } else if(strcmp(datacons_tag(v), "Empty") == 0) {
      f_empty(r, v);
    } else if(strcmp(datacons_tag(v), "Flip") == 0) {
      f_flip(r, v);
    } else if(strcmp(datacons_tag(v), "Overlay") == 0) {
      f_overlay(r, v);
    } else if(strcmp(datacons_tag(v), "Rotate") == 0) {
      f_rotate(r, v);
    } else {
      error("flange: Unknown data constructor: '%s'.\n", datacons_tag(v));
    }
    break;
  }
}

/****************************************/
/* Top-level: render a FLANGE picture. */

void
flange_render(int type_check, graphics_t *g, program_t *program,
              value_t *(*evaluate_program)(env_t *env, program_t *program))
{
  list_t *fts = flange_types();
  env_t *env = flange_bindings();
  type_t *type_of_main = NULL;
  render_t r;
  double width = 600, height = 600; /* FIXME */

  /* Append the FLANGE-specific types. */
  list_append(&program->types, &fts);

  if(type_check && !(type_of_main = types_check(program))) {
    fprintf(stdout, " ** Aborting on type error.\n");
    exit(1);
  }

  /* Assumes the bottom-left coordinate is (0, 0) and top-right is
   * (width, height). */
  r.g = g;
  r.origin.x = 0;
  r.origin.y = 0;

  r.bottom.x = width;
  r.bottom.y = 0;

  r.left.x = 0;
  r.left.y = height;

  /* Draw the picture. */
  if( type_of_main == NULL
      || (type_of_main->type == t_user_defined
          && strcmp(user_def_use_name(type_of_main), "Picture") == 0) ) {
    graphics_picture_start(g, (int)width, (int)height);
    f_render(&r, evaluate_program(env, program));
    graphics_picture_end(g);
  } else {
    print_value(stdout, evaluate_program(builtin_bindings(), program));
    fprintf(stdout, "\n");
  }
}
