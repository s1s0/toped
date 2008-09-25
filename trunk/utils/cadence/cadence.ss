(require scheme/list)
;****************AUX FUNCTIONS********************************

;bin-str->hex-str
;input str - string containing binary representation (e.g. "010101"). Warning: no need to put "#b" before string
;output - string containing hex representation (e.g. "0x15")
(define (bin-str->hex-str str)
  (string-append "0x"(number->string (string->number (string-append "#b" str)) 16)))

;expand-to-N
;input itemlist - list of items, N - length of required sequences
;output - list of bits that expand to N items (repeated initial list);
;warning! List that longer then N items will be cut
(define (expand-to-N itemlist N)
  (define (expand-to-1 l)
    (let ((itempos (remainder (length l) (length itemlist))))
      (if (= (length l) N)
        l
        (expand-to-1 (append l (list (list-ref itemlist itempos)))))))
  (if (>= (length itemlist) N) 
      (take itemlist N)
      (expand-to-1 itemlist)))

;expand-string-to-32
;input str - string of bits
;output - string of bits expanded to 32 bits 
(define (expand-string-to-32 str)
  (list->string (expand-to-N (string->list str)32)))

;split-to-8
;input str - string
;output - list of strings where every string constains 8 characters
(define (split-to-8 str)
  (if (= (string-length str) 0) '()
      (append (list (substring str 0 8)) (split-to-8 (substring str 8 (string-length str))))))

;map-exclude-last
; input func -function of 1 argument
;       lst  - list
;Output - regular map for all elements except last one, last is returned without changes
;Note - this function is necessary to add "," after all bytes in drDefineStipple except last one
(define (map-exclude-last func lst)
  (append (map func (take lst (- (length lst) 1)))
          (list (last lst))))
;list-of-lists->list
;input lst - list of lists
;output - list containing all elements from lst
;Warning - work only  down to one level
(define (list-of-lists->list lst)
  (foldl (lambda(x result) (append result x)) '() lst))
;(expand-to-N (list 0 1 1 1) 32)
;(expand-string-to-32 "0111")
;(split-to-8 "0123456701234567")
;(map-exclude-last (lambda(x) (+ x 10)) '(1 2 3))
;(list-of-lists->list (list (list 1 2 3) (list 3 4 5)))
;****************CADENCE API FUNCTIONS********************************
;---------------------------------------------
(define drDefineDisplay 
  (lambda(f) (list "/*drDefineDisplay"
                   "toped doesn't support this command*/")))


;-----------------------------------------------
(define drDefineColor
  (lambda(colorlist) 
    (append (list "void ColorSetup() {")
          (map (lambda (x)
                 (let ((color (cadr x))
                       (red (caddr x))
                       (green (cadddr x))
                       (blue (cadr (cdddr x))))
                   (string-append "definecolor(\""  (symbol->string color) "\","
                                  (number->string red) ","
                                  (number->string green) ","
                                  (number->string blue) ", 178);"))) 
               colorlist)
          (list "}"))))  

;---------------------------------------------
(define drDefineStipple
  (lambda(stipplelist)
    (let ((l '()))
      (append (list "void fillSetup() {") 
              (map (lambda(stipple)
                          (let ((display (car stipple))
                                (name (cadr stipple))
                                (bitmap (caddr stipple)))
                            (set! l (append l (list name)))
                            (append 
                             (list "int list " name "= {" )
                                   (map-exclude-last (lambda (str) (string-append str ","))
                                    (foldl (lambda (word result) 
                                          (append result  ;(map-exclude-last 
                                                          ; (lambda (str) (string-append str ","))
                                                           (map bin-str->hex-str 
                                                          (split-to-8 (expand-string-to-32
                                                           ;get list of bits from cadence bitmap
                                                           (foldl 
                                                            (lambda (bit bitstr)
                                                              (string-append bitstr (number->string bit))) 
                                                            ""  word)
                                            )))
                                                           ;)
                                                  ))
                                          '()
                                        bitmap))
                                   (list "};"))
                            )
                     )
                   stipplelist) 
              (list "}")
              
))))
          ;    (for-each 
          ;     (lambda(x) (string-append (bin-str->hex-str x)))

;****************TRANSLATOR FUNCTIONS********************************
;----------------------------------------------
(define (readlines filename)
  (call-with-input-file filename
    (lambda (p)
      (let loop ((line (read p)) 
                 (result '()))
        (if (eof-object? line)
            (reverse result)
            (loop (read p) (cons line result)))))))

;---------------------------------------------
(define (parse lst) 
  (let ((command (car lst))
        (body (cadr lst)))
    (cons (cond ((eq? command 'drDefineColor) (drDefineColor body))
          ((eq? command 'drDefineDisplay) (drDefineDisplay body))
          ((eq? command 'drDefineStipple) (drDefineStipple body))
          (else (begin
                  (display "mistake in recognition")
                  (newline)
                  (display "don't know ")
                  (display command)
                  (newline)
                  '())))
          (if (eq? (cddr lst) '())
              '()
              (parse (cddr lst))))))
;---------------------------------------------
(define (write-to-file filename list-of-string)
  (call-with-output-file filename
  (lambda (p)
    (for-each (lambda (s) 
                (cond ((empty? s) #f)
                      ((not (list? s)) (display s p) (newline))
                      (else (for-each (lambda (x) (if (list? x)
                                                      (begin
                                                        (for-each (lambda(y) (display y p) ) x)
                                                        (newline p))
                                                      (begin 
                                                        (display x p) 
                                                        (newline p))))
                                      s))))
              list-of-string))
  'replace))


;---------------------------------------------
(define d (readlines "default.drf"))
(write-to-file "tell.tll" (parse d))


(parse d)
