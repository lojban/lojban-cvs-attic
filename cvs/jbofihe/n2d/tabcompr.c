/***************************************
  $Header$

  Routines to compress the DFA transition tables, by identifying where two DFA
  states have a lot of transitions the same.
  ***************************************/

/* COPYRIGHT */

#include "n2d.h"

/* ================================================================= */
/* Treat 'x' as a set of 16 bit pairs, with field (0..15) specifying
   which.  Increment the field'th bit pair as a gray code, in the
   pattern 00->01->11->10->00 */

unsigned long increment(unsigned long x, int field)
{
  int f2 = field + field;
  static unsigned char transxor[4] = {1, 2, 2, 1};
  unsigned long g = x >> f2;
  unsigned long h = transxor[g&3];
  return x ^ (h<<f2);
}

/* ================================================================= */
/* Calculate the number of bits set in an unsigned long. */

unsigned long count_bits_set(unsigned long x)
{
  unsigned long y = x;
  unsigned long c;
  c = 0x55555555UL;
  y = ((y>>1) & c) + (y & c);
  c = 0x33333333UL;
  y = ((y>>2) & c) + (y & c);
  y = (y>>4) + y;
  c = 0x0f0f0f0fUL;
  y &= c;
  y = (y>>8) + y;
  y = (y>>16) + y;
  return y & 0x1f;
}

/* ================================================================= */
/* Compute 'signatures' of the transitions out of a particular state.
   The signature is given by considering the destination state numbers mod 16,
   and counting how many transitions there are in each resulting equivalence
   class.  The number is encoded using the gray code implied by the increment
   fn. */

static void
compute_transition_sigs(DFANode **dfas, int ndfas, int ntokens)
{
  int i, j;
  for (i=0; i<ndfas; i++) {
    unsigned long ts = 0UL; /* transition signature */
    for (j=0; j<ntokens; j++) {
      unsigned long dest = dfas[i]->map[j];
      dest &= 0xf; /* 16 bit pairs in 'ts' */
      ts = increment(ts, dest);
    }
    dfas[i]->transition_sig = ts;
  }
}


/* ================================================================= */

#define REQUIRED_BENEFIT 2

static void
find_default_states(DFANode **dfas, int ndfas, int ntokens)
{
  int i, j, t;
  int best_index;
  int best_diff;
  int trans_count; /* Number of transitions in working state */
  unsigned long tsi;

  for (i=0; i<ndfas; i++) {
    trans_count = 0;
    for (t=0; t<ntokens; t++) {
      if (dfas[i]->map[t] >= 0) trans_count++;
    }
  
    dfas[i]->defstate = -1; /* not defaulted */
    best_index = -1;
    best_diff = ntokens + 1; /* Worse than any computed value */
    tsi = dfas[i]->transition_sig;
    for (j=0; j<i; j++) {
      unsigned long tsj;
      unsigned long sigdiff;
      int diffsize;

      if (dfas[j]->defstate >= 0) continue; /* Avoid chains of defstates */
      tsj = dfas[j]->transition_sig;

      /* This is the heart of the technique : if we xor two vectors of bit
         pairs encoded with the gray code above, and count the number of bits
         set in the result, we get the sum of absolute differences of the bit
         pairs.   The number of outgoing transitions that differ between the
         states must be _at_least_ this value.  It may in fact be much greater
         (i.e. we may get 'false matches').  However, this algorithm is a quick
         way of filtering most of the useless potential default states out. */
      
      sigdiff = tsi ^ tsj;
      diffsize = count_bits_set(sigdiff);
      if (diffsize >= best_diff) continue;
      if (diffsize >= trans_count) continue; /* Else pointless! */

      /* Otherwise, do an exact check (i.e. see how much false matching we
         suffered). */
      diffsize = 0;
      for (t=0; t<ntokens; t++) {
        if (dfas[i]->map[t] != dfas[j]->map[t]) {
          diffsize++;
        }
      }

      if (((best_index < 0) || (diffsize < best_diff))
          &&
          (diffsize < (trans_count - REQUIRED_BENEFIT))) {
        best_index = j;
        best_diff = diffsize;
      }
    }

    dfas[i]->defstate = best_index;
    dfas[i]->best_diff = best_diff;
  }
}

/* ================================================================= */

void
compress_transition_table(DFANode **dfas, int ndfas, int ntokens)
{
  compute_transition_sigs(dfas, ndfas, ntokens);
  find_default_states(dfas, ndfas, ntokens);
}

/* ================================================================= */

#ifdef TEST
int main () {
  unsigned long x = 0;
  unsigned long x1, x2, x3, x4;
  x1 = increment(x,  2);
  x2 = increment(x1, 2);
  x3 = increment(x2, 2);
  x4 = increment(x3, 2);
  printf("%d %d %d %d %d\n", x, x1, x2, x3, x4);

  printf("1=%d\n", count_bits_set(0x00000001));
  printf("2=%d\n", count_bits_set(0x00000003));
  printf("3=%d\n", count_bits_set(0x00000007));
  printf("4=%d\n", count_bits_set(0x0000000f));
  printf("4=%d\n", count_bits_set(0xf0000000));
  
  return 0;
}
#endif

  
