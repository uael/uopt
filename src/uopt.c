/*
 * MIT License
 *
 * Copyright (c) 2016-2017 Abel Lucas <www.github.com/uael>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "uopt.h"

void
opts_ctor(opts_t *self, opt_t *opts, optcb_t callback) {
  u32_t it;

  *self = (opts_t) {
    .callback = callback
  };
  err_stack_ctor(&self->errs);
  if (opts) {
    while (opts->lf) {
      opt_t *opt = opts++;

      if (optmap_put(&self->conf, opt->lf, &it) == RET_SUCCESS) {
        self->conf.vals[it] = *opt;
        opt = self->conf.vals + it;
      }
      if (opt->f) {
        if (optmap_sc_put(&self->shortcuts, opt->f, &it) == RET_SUCCESS) {
          self->shortcuts.vals[it] = opt;
        }
      }
    }
  }
}

void
opts_dtor(opts_t *self) {
  optmap_dtor(&self->conf);
  optmap_sc_dtor(&self->shortcuts);
  err_stack_dtor(&self->errs);
}

ret_t
opts_parse(opts_t *self, void *app_ptr, i32_t argc, i8_t **argv) {
  i8_t *arg, key, *lkey, *val, errmsg[U8_MAX];
  opt_t *opt;
  i32_t i;

  if (argc) {
    if ((self->program = strrchr(argv[0], '/'))) {
      ++self->program;
    } else {
      self->program = argv[0];
    }
    for (i = 1; i < argc; ++i) {
      arg = argv[i];
      val = nil;
      if (*arg == '-') {
        if (*(arg + 1) == '-') {
          opt = opts_lget(self, lkey = arg + 2);
          if (opt == nil) {
            sprintf(errmsg, "unrecognized command line option ‘%s’", lkey);
            goto opts_warning;
          }
          if (opt->kval) {
            if (i < argc - 1) {
              val = argv[++i];
            } else {
              sprintf(errmsg, "missing argument for command line option ‘%s’",
                lkey
              );
              goto opts_error;
            }
            if (!opt->global && opt->match) {
              sprintf(errmsg,
                "duplicate value for command line option ‘%s’: ‘%s’", lkey, val
              );
              goto opts_warning;
            }
          } else if (!opt->global && opt->match) {
            sprintf(errmsg, "duplicate command line option ‘%s’", lkey);
            goto opts_warning;
          }
        } else {
          opt = opts_get(self, key = arg[1]);
          if (opt == nil) {
            sprintf(errmsg, "unrecognized command line option ‘%c’", key);
            goto opts_warning;
          }
          if (opt->kval) {
            if (arg[2] != '\0') {
              val = arg + 2;
            } else if (i < argc - 1) {
              val = argv[++i];
            } else {
              sprintf(errmsg, "missing argument for command line option ‘%c’",
                key
              );
              goto opts_error;
            }
            if (!opt->global && opt->match) {
              sprintf(errmsg,
                "duplicate value for command line option ‘%c’: ‘%s’", key, val
              );
              goto opts_warning;
            }
          } else if (arg[2] != '\0') {
            sprintf(errmsg, "unrecognized command line option ‘%c’", key);
            goto opts_warning;
          } else if (!opt->global && opt->match) {
            sprintf(errmsg, "duplicate command line option ‘%c’", key);
            goto opts_warning;
          }
        }
        if (opt->callback == nil || opt->callback(app_ptr, val) == 0) {
          opt->match = true;
        }
      } else if (self->callback && (self->callback(app_ptr, arg) != 0)) {
        sprintf(errmsg, "invalid command line argument ‘%s’", arg);
        goto opts_error;
      }
      continue;
      opts_warning:
      if (err_stack_push(&self->errs, err_usr(ERRLVL_WARNING, errmsg))
        == RET_ERRNO) {
        return RET_ERRNO;
      }
      continue;
      opts_error:
      if (err_stack_push(&self->errs, err_usr(ERRLVL_ERROR, errmsg))
        == RET_ERRNO) {
        return RET_ERRNO;
      }
    }
  }
  return self->errs.len ? RET_FAILURE : RET_SUCCESS;
}
