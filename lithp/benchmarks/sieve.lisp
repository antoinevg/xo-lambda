(defun range (min max) (loop for i from min below max collect i))
(defun sieve (num) 
  (loop for ints = (range 2 num) 
        then (remove-if (lambda (x) (= 0 (mod x (first ints)))) ints)
        until (null ints) collect (first ints)))
(sieve 10)

