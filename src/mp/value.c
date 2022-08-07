/* Copyright 2021-2025 MarcosHCK
 * This file is part of libabaco.
 *
 * libabaco is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libabaco is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libabaco.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <config.h>
#include <glib-object.h>
#include <value.h>

typedef struct _MpValue MpValue;

typedef enum
{
  MP_TYPE_NIL = 0,  /* empty slot       */
  MP_TYPE_VALUE,    /* arbitrary value  */
  MP_TYPE_INTEGER,
  MP_TYPE_RATIONAL,
  MP_TYPE_REAL,
  MP_TYPE_MAX,
} MpType;

struct _MpValue
{
  MpType type;
  union
  {
    GValue value;
    mpz_t integer;
    mpq_t rational;
    mpfr_t real;
  };
};

struct _MpStack
{
  union
  {
    GArray array_;
    struct
    {
      MpValue* values;
      guint length;
    };
  };
};

static void
_mp_stack_notify (gpointer pvalue)
{
  MpValue* value = pvalue;
  switch (value->type)
  {
  case MP_TYPE_VALUE:
    g_value_unset (& value->value);
    break;
  case MP_TYPE_INTEGER:
    mpz_clear (value->integer);
    break;
  case MP_TYPE_RATIONAL:
    mpq_clear (value->rational);
    break;
  case MP_TYPE_REAL:
    mpfr_clear (value->real);
    break;
  }
}

static inline MpValue*
_mp_stack_index (MpStack* stack, int index)
{
  if (index >= 0 && stack->length < index)
    return & stack->values [index];
return NULL;
}

MpStack*
_mp_stack_new ()
{
  const long size = sizeof (MpValue);
  const long alloc = (1 << 4);
  GArray* array = NULL;

  array =
  g_array_sized_new (FALSE, TRUE, size, alloc);
  g_array_set_clear_func (array, _mp_stack_notify);
return (MpStack*) array;
}

gpointer
_mp_stack_ref (gpointer stack)
{
  g_return_val_if_fail (stack != NULL, NULL);
  g_array_ref (stack);
return stack;
}

void
_mp_stack_unref (gpointer stack)
{
  g_return_if_fail (stack != NULL);
  g_array_unref (stack);
}

const gchar*
_mp_stack_type (MpStack* stack, int index)
{
  g_return_val_if_fail (stack != NULL, NULL);
  GArray* array = (gpointer) stack;

  g_return_val_if_fail (index >= 0 && stack->length > index, NULL);
  MpValue* pmp = & stack->values [index];

  static const gchar** table = NULL;
  if (g_once_init_enter (& table))
  {
    static const gchar* __table__ [MP_TYPE_MAX];
    __table__ [MP_TYPE_NIL] = g_intern_static_string ("nil");
    __table__ [MP_TYPE_VALUE] =   g_intern_static_string ("value");
    __table__ [MP_TYPE_INTEGER] = g_intern_static_string ("integer");
    __table__ [MP_TYPE_RATIONAL] = g_intern_static_string ("rational");
    __table__ [MP_TYPE_REAL] = g_intern_static_string ("real");
    g_once_init_leave (& table, __table__);
  }
return table [pmp->type];
}

void
_mp_stack_transfer (MpStack* dst, MpStack* src)
{
  g_return_if_fail (src != NULL);
  g_return_if_fail (dst != NULL);
  GArray* asrc = (gpointer) src;
  GArray* adst = (gpointer) dst;

  if (src->length > 0)
  {
    guint last = src->length - 1;
    MpValue* pmp = & src->values [last];
    MpValue mp = *pmp;

    pmp->type = MP_TYPE_NIL;
    g_array_remove_index (asrc, last);
    g_array_append_val (adst, mp);
  }
}

void
_mp_stack_push_index (MpStack* stack, int index)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;

  g_return_if_fail (index >= 0 && stack->length > index);
  MpValue* pmp = & stack->values [index];
  MpValue mp = {0};

  switch (pmp->type)
  {
  case MP_TYPE_NIL:
    g_array_append_vals (array, pmp, 1);
    break;
  case MP_TYPE_VALUE:
    mp.type = MP_TYPE_VALUE;
    g_value_init (& mp.value, G_VALUE_TYPE (&pmp->value));
    g_value_copy (& pmp->value, & mp.value);
    g_array_append_val (array, mp);
    break;
  case MP_TYPE_INTEGER:
    mp.type = MP_TYPE_INTEGER;
    mpz_init (mp.integer);
    mpz_set (mp.integer, pmp->integer);
    g_array_append_val (array, mp);
    break;
  case MP_TYPE_RATIONAL:
    mp.type = MP_TYPE_RATIONAL;
    mpq_init (mp.rational);
    mpq_set (mp.rational, pmp->rational);
    g_array_append_val (array, mp);
    break;
  case MP_TYPE_REAL:
    mp.type = MP_TYPE_REAL;
    {
      mpfr_rnd_t mode;
      mode = mpfr_get_default_rounding_mode ();
      mpfr_init_set (mp.real, pmp->real, mode);
      g_array_append_val (array, mp);
    }
    break;
  default:
    g_warning ("Can't copy this value");
    break;
  }
}

void
_mp_stack_exchange (MpStack* stack, int index)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;

  g_return_if_fail (index >= 0);
  g_return_if_fail (stack->length > index);

  if (stack->length > 0)
  {
    guint last = stack->length - 1;
    MpValue* pmp1 = & stack->values [last];
    MpValue* pmp2 = & stack->values [index];
    MpValue tmp = *pmp1;

    *pmp1 = *pmp2;
    *pmp2 = tmp;
  }
}

void
_mp_stack_insert (MpStack* stack, int index)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;

  g_return_if_fail (index >= 0);
  if (stack->length > 0)
  {
    guint last = stack->length - 1;
    MpValue* pmp = & stack->values [last];
    MpValue mp = *pmp;

    pmp->type = MP_TYPE_NIL;
    g_array_remove_index (array, last);
    g_array_insert_val (array, index, mp);
  }
}

void
_mp_stack_remove (MpStack* stack, int index)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;

  g_return_if_fail (index >= 0 && stack->length > index);
  g_array_remove_index (array, index);
}

void
_mp_stack_new_integer (MpStack* stack)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  MpValue mp = { .type = MP_TYPE_INTEGER };
  mpz_init (mp.integer);
  g_array_append_val (array, mp);
}

void
_mp_stack_new_rational (MpStack* stack)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  MpValue mp = { .type = MP_TYPE_RATIONAL };
  mpq_init (mp.rational);
  g_array_append_val (array, mp);
}

void
_mp_stack_new_real (MpStack* stack)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  MpValue mp = { .type = MP_TYPE_REAL };
  mpfr_init (mp.real);
  g_array_append_val (array, mp);
}

void
_mp_stack_push_nil (MpStack* stack)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  MpValue mp = { .type = MP_TYPE_NIL };
  g_array_append_val (array, mp);
}

void
_mp_stack_push_value (MpStack* stack, const GValue* value)
{
  g_return_if_fail (stack != NULL);
  g_return_if_fail (G_IS_VALUE (value));
  GArray* array = (gpointer) stack;
  MpValue mp = {0};

  mp.type = MP_TYPE_VALUE;
  g_value_init (& mp.value, G_VALUE_TYPE (value));
  g_value_copy (value, & mp.value);
  g_array_append_val (array, mp);
}

gboolean
_mp_stack_push_string (MpStack* stack, const gchar* value, int base)
{
  g_return_val_if_fail (stack != NULL, FALSE);
  g_return_val_if_fail (base > 0, FALSE);
  GArray* array = (gpointer) stack;
  MpValue mp;

  gsize length = strlen (value);
  const gchar* end = NULL;
  g_return_val_if_fail (g_utf8_validate (value, length, &end), FALSE);
  g_return_val_if_fail (end - value == length, FALSE);
  g_return_val_if_fail (length > 0, FALSE);

  const gchar* val = value;
  const long bufsz = 512;
  gchar stat [bufsz + sizeof (void*)];
  gunichar c = 0;

  do
  {
    c = g_utf8_get_char (val);
    if (c == 0)
      break;

    switch (c)
    {
    case '.':
      mp.type = MP_TYPE_RATIONAL;
      {
        mpq_init (mp.rational);
        mpz_ptr num = mpq_numref (mp.rational);
        mpz_ptr den = mpq_denref (mp.rational);
        guint t, partial = 0;
        gchar* next = NULL;
        gchar* buf = NULL;
        int result;

        /* numerator */

        partial = length - g_utf8_skip [*(guchar*) val];
        next = g_utf8_next_char (val);
        if (partial > bufsz)
          buf = g_malloc (partial + 1);
        else
          buf = & stat [0];

        t = val - value;
        memcpy (& buf [0], value, t);
        memcpy (& buf [t], next, partial - t);
        buf [partial] = '\0';

        result = mpz_set_str (num, buf, base);
        if (G_UNLIKELY (result < 0))
        {
          if (buf != & stat [0])
            g_free (buf);
          return FALSE;
        }

        /* denominator */

        partial = g_utf8_strlen (val, -1);
        if (buf != & stat [0])
          buf = g_realloc (buf, partial + 1);
        else
        {
          if (partial > bufsz)
            buf = g_malloc (partial + 1);
        }

        memset (& buf [1], '0', partial - 1);
        buf [partial] = '\0';
        buf [0] = '1';

        result = mpz_set_str (den, buf, base);
        if (G_UNLIKELY (result < 0))
        {
          if (buf != & stat [0])
            g_free (buf);
          return FALSE;
        }

        /* canonicalize */

        if (buf != & stat [0])
          g_free (buf);
        mpq_canonicalize (mp.rational);
        g_array_append_val (array, mp);
        return TRUE;
      }
      break;
    case '/':
      mp.type = MP_TYPE_RATIONAL;
      {
        mpq_init (mp.rational);
        mpq_set_str (mp.rational, value, base);
        mpq_canonicalize (mp.rational);
        g_array_append_val (array, mp);
        return TRUE;
      }
      break;
    }
  } while ((val = g_utf8_next_char (val)) != NULL);

  mp.type = MP_TYPE_INTEGER;
  mpz_init (mp.integer);
  mpz_set_str (mp.integer, value, base);
  g_array_append_val (array, mp);
return TRUE;
}

void
_mp_stack_push_double (MpStack* stack, double value)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  mpfr_rnd_t mode;

  MpValue mp;
  mp.type = MP_TYPE_REAL;
  mode = mpfr_get_default_rounding_mode ();
  mpfr_init_set_d (mp.real, value, mode);
  g_array_append_val (array, mp);
}

void
_mp_stack_push_ldouble (MpStack* stack, long double value)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;
  mpfr_rnd_t mode;

  MpValue mp;
  mp.type = MP_TYPE_REAL;
  mode = mpfr_get_default_rounding_mode ();
  mpfr_init_set_ld (mp.real, value, mode);
  g_array_append_val (array, mp);
}

gpointer
_mp_stack_peek (MpStack* stack, int index)
{
  g_return_if_fail (stack != NULL);
  g_return_if_fail (index >= 0 && stack->length > index);
  MpValue* pmp = & stack->values [index];
return (gpointer) & pmp->integer;
}

void
_mp_stack_peek_value (MpStack* stack, int index, GValue* value)
{
  g_return_if_fail (stack != NULL);
  g_return_if_fail (index >= 0 && stack->length > index);
  MpValue* pmp = & stack->values [index];
  GArray* array = (gpointer) stack;

  switch (pmp->type)
  {
  case MP_TYPE_VALUE:
    g_value_init (value, G_VALUE_TYPE (& pmp->value));
    g_value_copy (& pmp->value, value);
    break;
  default:
    g_warning ("Can't cast to value");
    break;
  }
}

gchar*
_mp_stack_peek_string (MpStack* stack, int index, int base)
{
  g_return_val_if_fail (stack != NULL, NULL);
  g_return_val_if_fail (index >= 0 && stack->length > index, NULL);
  g_return_val_if_fail (base != 0, NULL);
  MpValue* pmp = & stack->values [index];
  GArray* array = (gpointer) stack;
  gchar* result = NULL;
  gsize length = 0;

  switch (pmp->type)
  {
  case MP_TYPE_INTEGER:
    length = 2;
    length += mpz_sizeinbase (pmp->integer, base);
    result = mpz_get_str (g_malloc (length), base, pmp->integer);
    break;
  case MP_TYPE_RATIONAL:
    length = 3;
    length += mpz_sizeinbase (mpq_numref (pmp->rational), base);
    length += mpz_sizeinbase (mpq_denref (pmp->rational), base);
    result = mpq_get_str (g_malloc (length), base, pmp->rational);
    break;
  case MP_TYPE_REAL:
    {
      mpfr_exp_t exp;
      mpfr_rnd_t mode;
      gsize partial = 0;
      gchar* tmp = NULL;

      mode = mpfr_get_default_rounding_mode ();
      tmp = mpfr_get_str (NULL, &exp, base, 0, pmp->real, mode);

      if (G_UNLIKELY (tmp == NULL))
      {
        g_warning ("invalid base");
        return NULL;
      }

      length = strlen (tmp);

      if (exp >= 0)
      {
        partial = ((length > exp) ? length : exp + 1) + 1;
        if (exp != 0)
          result = g_malloc (partial + 1);
        else
        {
          result = g_malloc (partial + 2);
          result [0] = '0';
          ++result;
        }

        memset (result, '0', partial);
        memcpy (& result [      0], & tmp [  0], (length > exp) ?          exp : length);
        memcpy (& result [exp + 1], & tmp [exp], (length > exp) ? length - exp :      0);
        result [partial] = '\0';
        result [exp] = '.';

        if (exp == 0)
          --result;
      } else
      if (exp < 0)
      {
        exp = -exp;
        partial = length + exp + 2;
        result = g_malloc (partial + 1);

        memset (result, '0', partial);
        memcpy (& result [exp + 2], tmp, length);
        result [partial] = '\0';
        result [1] = '.';
      }
      else
      {
        mpfr_free_str (tmp);
        g_error ("WTF?");
        g_assert_not_reached ();
      }

      mpfr_free_str (tmp);
    }
    break;
  default:
    g_warning ("Can't cast to string");
    break;
  }
return result;
}

double
_mp_stack_peek_double (MpStack* stack, int index)
{
  g_return_val_if_fail (stack != NULL, 0);
  g_return_val_if_fail (index >= 0 && stack->length > index, 0);
  MpValue* pmp = & stack->values [index];
  GArray* array = (gpointer) stack;
  double result = 0;

  switch (pmp->type)
  {
  case MP_TYPE_INTEGER:
    result = mpz_get_d (pmp->integer);
    break;
  case MP_TYPE_RATIONAL:
    result = mpq_get_d (pmp->rational);
    break;
  case MP_TYPE_REAL:
    {
      mpfr_rnd_t mode;
      mode = mpfr_get_default_rounding_mode ();
      result = mpfr_get_d (pmp->real, mode);
    }
    break;
  default:
    g_warning ("Can't cast to double");
    break;
  }
return result;
}

long double
_mp_stack_peek_ldouble (MpStack* stack, int index)
{
  g_return_val_if_fail (stack != NULL, 0);
  g_return_val_if_fail (index >= 0 && stack->length > index, 0);
  MpValue* pmp = & stack->values [index];
  GArray* array = (gpointer) stack;
  long double result = 0;

  switch (pmp->type)
  {
  case MP_TYPE_REAL:
    {
      mpfr_rnd_t mode;
      mode = mpfr_get_default_rounding_mode ();
      result = mpfr_get_ld (pmp->real, mode);
    }
    break;
  default:
    g_warning ("Can't cast to double");
    break;
  }
return result;
}

void
_mp_stack_pop (MpStack* stack, guint count)
{
  g_return_if_fail (stack != NULL);
  GArray* array = (gpointer) stack;

  if (stack->length >= count)
  {
    guint leave = stack->length - count;
    g_array_set_size (array, leave);
  }
}
