(set-logic QF_BV)
(set-info :smt-lib-version 2.0)
(echo "This is simple Sign Extend test")
(declare-fun x () (_ BitVec 2))
; sext1(x[2:]) < 0[3:] xor x[2:] < 0[2:]
; sext1(x) < 0 is equivalence to x < 0
; This proposition must be true. Result of oppsition should become unsat (i.e. no counter example).
(assert (xor (bvslt ((_ sign_extend 1) x) (_ bv0 3)) (bvslt x (_ bv0 2))))
(exit)
