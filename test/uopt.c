/*
 * libil - Intermediate Language cross-platform c library
 * Copyright (C) 2016-2017 Lucas Abel <www.github.com/uael>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>
 */

#include <cute.h>
#include <uds/vec.h>

#include "uopt.h"

#define STRNSIZE(s) (s), (sizeof(s)-1)

typedef struct my_app my_app_t;

struct my_app {
  bool_t echo, pp;
  i8_t const *output;
  strvec_t inputs;
};

ret_t
my_app_echo(my_app_t *app, UNUSED i8_t const *val) {
  app->echo = true;
  return RET_SUCCESS;
}

ret_t
my_app_pp(my_app_t *app, UNUSED i8_t const *val) {
  app->pp = true;
  return RET_SUCCESS;
}

ret_t
my_app_set_output(my_app_t *app, i8_t const *val) {
  app->output = val;
  return RET_SUCCESS;
}

ret_t
my_app_add_input(my_app_t *app, i8_t const *val) {
  return strvec_push(&app->inputs, (i8_t *) val);
}

CUTEST_DATA {
  my_app_t my_app;
  opts_t opts;
};

CUTEST_SETUP {
  static opt_t opts[4] = {
    {'S', "prepossess", 
      "output the prepossessed input file", (optcb_t) my_app_pp},
    {0, "echo", 
      "print the content of the input file", (optcb_t) my_app_echo},
    {'o', "output", 
      "output file name", (optcb_t) my_app_set_output, true},
    {0, nil}
  };
  self->my_app = (my_app_t) {0};
  strvec_ctor(&self->my_app.inputs);
  opts_ctor(&self->opts, opts, (optcb_t) my_app_add_input);
}

CUTEST_TEARDOWN {
  strvec_dtor(&self->my_app.inputs);
  opts_dtor(&self->opts);
}

CUTEST(opt, input);
CUTEST(opt, catval);
CUTEST(opt, mmatch);
CUTEST(opt, unrecognized);
CUTEST(opt, missing1);
CUTEST(opt, missing2);
CUTEST(opt, duplicate);

int
main(void) {
  CUTEST_DATA test = {0};

  CUTEST_PASS(opt, input);
  CUTEST_PASS(opt, catval);
  CUTEST_PASS(opt, mmatch);
  CUTEST_PASS(opt, unrecognized);
  CUTEST_PASS(opt, missing1);
  CUTEST_PASS(opt, missing2);
  CUTEST_PASS(opt, duplicate);
  return EXIT_SUCCESS;
}

CUTEST(opt, input) {
  opts_parse(&self->opts, &self->my_app, 5, (char *[5]) {"cli", "-o", "bla", "-S", "test/opt.c"});
  ASSERT(self->my_app.output && memcmp("bla", self->my_app.output, 3) == 0);
  ASSERT(self->my_app.pp == true);
  return CUTE_SUCCESS;
}

CUTEST(opt, catval) {
  opts_parse(&self->opts, &self->my_app, 2, (char *[2]) {"cli", "-obla"});
  ASSERT(self->my_app.output && memcmp("bla", self->my_app.output, 3) == 0);
  return CUTE_SUCCESS;
}

CUTEST(opt, mmatch) {
  opts_parse(&self->opts, &self->my_app, 5, (char *[5]) {"cli", "--output", "bla", "--echo", "--prepossess"});
  ASSERT(self->my_app.output && memcmp("bla", self->my_app.output, 3) == 0);
  ASSERT(self->my_app.echo == true);
  ASSERT(self->my_app.pp == true);
  return CUTE_SUCCESS;
}

CUTEST(opt, unrecognized) {
  err_t err;

  opts_parse(&self->opts, &self->my_app, 5, (char *[5]) {"cli", "--foo", "-foo", "-b", "--echo"});
  ASSERT(self->opts.errs.len == 3);
  err = self->opts.errs.buf[0];
  ASSERT(memcmp(err.msg, STRNSIZE("unrecognized command line option ‘foo’")) == 0);
  err = self->opts.errs.buf[1];
  ASSERT(memcmp(err.msg, STRNSIZE("unrecognized command line option ‘f’")) == 0);
  err = self->opts.errs.buf[2];
  ASSERT(memcmp(err.msg, STRNSIZE("unrecognized command line option ‘b’")) == 0);
  return CUTE_SUCCESS;
}

CUTEST(opt, missing1) {
  err_t err;

  opts_parse(&self->opts, &self->my_app, 2, (char *[2]) {"cli", "--output"});
  ASSERT(self->opts.errs.len == 1);
  err = self->opts.errs.buf[0];
  ASSERT(memcmp(err.msg, STRNSIZE("missing argument for command line option ‘output’")) == 0);
  return CUTE_SUCCESS;
}

CUTEST(opt, missing2) {
  err_t err;

  opts_parse(&self->opts, &self->my_app, 2, (char *[2]) {"cli", "-o"});
  ASSERT(self->opts.errs.len == 1);
  err = self->opts.errs.buf[0];
  ASSERT(memcmp(err.msg, STRNSIZE("missing argument for command line option ‘o’")) == 0);
  return CUTE_SUCCESS;
}

CUTEST(opt, duplicate) {
  err_t err;

  opts_parse(&self->opts, &self->my_app, 4, (char *[4]) {"cli", "-obla", "--echo", "-S"});
  opts_parse(&self->opts, &self->my_app, 8,
    (char *[8]) {"cli", "--output", "bla", "--echo", "-obla", "-o", "bla", "-S"});
  ASSERT(self->opts.errs.len == 5);
  err = self->opts.errs.buf[0];
  ASSERT(memcmp(err.msg, STRNSIZE("duplicate value for command line option ‘output’: ‘bla’")) == 0);
  err = self->opts.errs.buf[1];
  ASSERT(memcmp(err.msg, STRNSIZE("duplicate command line option ‘echo’")) == 0);
  err = self->opts.errs.buf[2];
  ASSERT(memcmp(err.msg, STRNSIZE("duplicate value for command line option ‘o’: ‘bla’")) == 0);
  err = self->opts.errs.buf[3];
  ASSERT(memcmp(err.msg, STRNSIZE("duplicate value for command line option ‘o’: ‘bla’")) == 0);
  err = self->opts.errs.buf[4];
  ASSERT(memcmp(err.msg, STRNSIZE("duplicate command line option ‘S’")) == 0);
  return CUTE_SUCCESS;
}