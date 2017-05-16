typedef enum {
  UNDEFINED,
  SYMBOL,
  BVCONST,
  BVNOT,
  BVCONCAT,
  BVOR,
  BVAND,
  BVXOR,
  BVNAND,
  BVNOR,
  BVXNOR,
  BVEXTRACT,
  BVLEFTSHIFT,
  BVRIGHTSHIFT,
  BVSRSHIFT,
  BVPLUS,
  BVSUB,
  BVUMINUS,
  BVMULT,
  BVDIV,
  BVMOD,
  SBVDIV,
  SBVREM,
  SBVMOD,
  BVSX,
  BVZX,
  ITE,
  BOOLEXTRACT,
  BVLT,
  BVLE,
  BVGT,
  BVGE,
  BVSLT,
  BVSLE,
  BVSGT,
  BVSGE,
  EQ,
  FALSE,
  TRUE,
  NOT,
  AND,
  OR,
  NAND,
  NOR,
  XOR,
  IFF,
  IMPLIES,
  PARAMBOOL,
  READ,
  WRITE,
  ARRAY,
  BITVECTOR,
  BOOLEAN
} ASTKind;

extern const char *_kind_names[];
