
; a pretty thing
;(set 'pi 3.141592)
;(set 'pretty '(lambda (n) 
;  (cond ((lte n 0.0) 0.0) 
;        ('t (progn 
;              (print n)
;              (pretty (sub n 0.1)))))))
;(set 'pretty '(label f (lambda (n) 
;  (cond ((lte n 0.0) 0.0) 
;        ('t (progn 
;              (print n)
;              (f (sub n 0.1))))))))
;(pretty (mul pi 2.0))


;((label subst (lambda (x y z) (cond ((atom z) (cond ((eq z y) x) ('t z))) ('t (cons (subst x y (car z)) (subst x y (cdr z))))))) 'm 'b '(a b (a b c) d))
;((label factorial (lambda (n) (cond ((eq n 0) 1) ('t (mul n (factorial (sub n 1))))))) 30)
(set '+ 'add)
(set '* 'mul)
(set '- 'sub)
(set '/ 'div)
;(set 'add1 '(lambda (x) (+ x 1)))
;(add1 2)
;(set 'factorial '(label f (lambda (x) (cond ((eq x 0) 1) ('t (* x (f (- x 1))))))))


(set 'factorial '(label factorial (lambda (x) (cond ((eq x 0) 1) ('t (* x (factorial (- x 1))))))))


;(set 'factorial '(label factorial (lambda (x) (cond ((eq x 0.0) 1.0) ('t (* x (factorial (- x 1.0))))))))
;(print "factor some numbers")
;(factorial 3)
;(factorial 3.0)
;(factorial 16)
;(factorial 17)
;(print "overflow")
;(factorial 34)  ; Why is the overflow evaluating to 0 ?
(factorial 33.0)
(factorial 33.0)
(xl:gc)
(factorial 33.0)
(factorial 33.0)
(xl:gc)
(factorial 33.0)
(factorial 33.0)
(xl:gc)
(factorial 33.0)
(factorial 33.0)
(xl:gc)
(factorial 33.0)
(factorial 33.0)
(xl:gc)
(factorial 33.0)
;(factorial 4.0)
;(factorial 4)
;(factorial 4.0)
;(factorial 100.0)

