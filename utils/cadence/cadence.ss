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

(define (layer-name->string name) 
  (cond ((symbol? name)(symbol->string name))
        ((string? name) name)
        (else (error "error in layer-name->string. Strange type for conversion"))))

;(expand-to-N (list 0 1 1 1) 32)
;(expand-string-to-32 "0111")
;(split-to-8 "0123456701234567")
;(map-exclude-last (lambda(x) (+ x 10)) '(1 2 3))
;(list-of-lists->list (list (list 1 2 3) (list 3 4 5)))


;****************OBJECT DEFINITIONS********************************
(define (define-packet name)
  (let ((stipple "")
        (line-style "")
        (fill "")
        (outline "")
        (fill-style ""))
    (define (set-stipple! stip) (set! stipple stip))
    (define (get-stipple) stipple)
    (define (set-line-style! style) (set! line-style style))
    (define (get-line-style) line-style)
    (define (set-fill! fl) (set! fill fl))
    (define (get-fill) fill)
    (define (set-outline! outln) (set! outline outln))
    (define (get-outline) outline)
    (define (set-fill-style! fl-style) (set! fill-style fl-style))
    (define (get-fill-style) fill-style)
    (define (get-name) name)
    (define (dispatch m)
      (let ()
        (cond ((eq? m 'set-stipple!) set-stipple!)
              ((eq? m 'get-stipple) get-stipple )
              ((eq? m 'set-line-style!) set-line-style!)
              ((eq? m 'get-line-style) get-line-style)
              ((eq? m 'set-fill!) set-fill!)
              ((eq? m 'get-fill) get-fill)
              ((eq? m 'set-outline!) set-outline!)
              ((eq? m 'get-outline) get-outline)
              ((eq? m 'set-fill-style!) set-fill-style!)
              ((eq? m 'get-fill-style) get-fill-style)
              ((eq? m 'get-name) get-name )
              (else (error "unknown method in define-paket")))))
    dispatch))

(define (define-layer name number)
  (let ((layer-name (layer-name->string name))
        (streamed #f)
        (stream-number #f)
        (data-type #f))
    (define (get-name) layer-name)
    (define (get-number) number)
    (define (get-streamed) streamed)
    (define (set-streamed! bool) (set! streamed bool))
    (define (get-stream-number) stream-number)
    (define (set-stream-number! num) (set! stream-number num))
    (define (get-data-type) data-type)
    (define (set-data-type! type) (set! data-type type))
    
    (define (dispatch m)
      (let ()
        (cond ((eq? m 'get-name) get-name)
              ((eq? m 'get-number) get-number)
              ((eq? m 'get-streamed) get-streamed)
              ((eq? m 'set-streamed!) set-streamed!)
              ((eq? m 'get-stream-number) get-stream-number)
              ((eq? m 'set-stream-number!) set-stream-number!)
              ((eq? m 'get-data-type) get-data-type)
              ((eq? m 'set-data-type!) set-data-type!)
              (else (error "unknown method in define-layer")))))
    dispatch))

;find-layer
;input name - string - name of layer
;output - function of corresponding layer or null
(define (find-layer layer-list name)
  (cond
    ((empty? layer-list) null)
    ((string=? (((car layer-list) 'get-name)) name) (car layer-list))
    (else (find-layer (cdr layer-list) name))))

;****************GLOBAL VARIABLES AND FUNCTIONS*******************
(define packet-list  '())
(define layer-list '())

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
                                                                        ""  word))))))
                                           '()
                                           bitmap))
                                   (list "};"))))
                   stipplelist) 
              (list "}")))))


(define drDefineLineStyle
  (lambda(style)
    (list "/*drLineStyle"
          "toped doesn't support this command*/"))) 

(define drDefinePacket
  (lambda(packets)
    (map (lambda(x)
           (let*  (
                   (temp (cddddr x))
                   (name (cadr x))
                   (stipple (caddr x))
                   (line-style (cadddr x))
                   (fill (car temp))
                   (outline (cadr temp))
                   (fill-style (caddr temp))
                   
                   (packet (define-packet name)))
                 (begin
                   ((packet 'set-stipple!) stipple)
                   ((packet 'set-line-style!) line-style)
                   ((packet 'set-fill!) fill)
                   ((packet 'set-outline!) outline)
                   ((packet 'set-fill-style!) fill-style)
                   packet))) 
         packets)))

(define layerDefinitions
  (lambda (definitions)
    (parse definitions)))

(define techPurposes
  (lambda(purposes)
    (list "/*techPurposes "
                   "toped doesn't support this command*/")))

(define techLayers
  (lambda(layers)
    (map
     (lambda (layer)
       (let* ( (name (car layer))
               (number (cadr layer))
               (new-layer (define-layer name number)))
         new-layer))
     layers)))

(define techLayerPurposePriorities
  (lambda(layers)
    (list "/*techLayerPurposePriorities "
                   "toped doesn't support this command*/")))

(define techDisplays
  (lambda(layers)
    (list "/*techDisplays "
                   "toped doesn't support this command*/")))

(define techLayerProperties
  (lambda(layers)
    (list "/*techLayerProperties "
                   "toped doesn't support this command*/")))

(define layerRules
  (lambda(rules)
    (parse rules)))

(define streamLayers
  (lambda(layers)
    (for-each 
     (lambda(cur-layer)
       (let* ((layer-name (car (car cur-layer)))
              (layer (find-layer layer-list layer-name))
              (stream-number (cadr cur-layer))
              (data-type (caddr cur-layer))
              (streamed (cadddr cur-layer)))
         (if (not (eq? layer null)) 
             (if (eq? streamed 't)
                 (begin 
                   ((layer 'set-streamed!) #t)
                   ((layer 'set-stream-number!) stream-number)
                   ((layer 'set-data-type!) data-type))))))
     layers)))

(define physicalRules
  (lambda (body)
    (parse body)))

(define mfgGridResolution
  (lambda (resolutions)
    (list "/*mfgGridResolution"
          "not realized yet*/")))

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
          ((eq? command 'drDefineLineStyle) (drDefineLineStyle body))
          ((eq? command 'drDefinePacket) (begin
                                           (set! packet-list (append  (drDefinePacket body) packet-list))
                                           '()))
          ((eq? command 'layerDefinitions) (layerDefinitions body))
          ((eq? command 'techPurposes) (techPurposes body))
          ((eq? command 'techLayers) (begin
                                           (set! layer-list (append  (techLayers body) layer-list))
                                           '()))
          ((eq? command 'techLayerPurposePriorities) (techLayerPurposePriorities body))
          ((eq? command 'techDisplays) (techDisplays body))
          ((eq? command 'techLayerProperties) (techLayerProperties body))
          ((eq? command 'layerRules) (layerRules body))
          ((eq? command 'streamLayers) (streamLayers body))
          ((eq? command 'physicalRules) (physicalRules body))
          ((eq? command 'mfgGridResolution) (mfgGridResolution body))
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
(define d (foldl (lambda (word result) 
                                          (append result (readlines word))) '() (list "default.drf" "techfile.tf"))); result (readlines "default.drf"))))
(write-to-file "tell.tll" (parse d))


;(parse d)
;(find-layer layer-list "PW")
;(find-layer layer-list "NBL1")
;((cadr layer-list) 'get-name)
(for-each 
 (lambda(layer)
   (begin
     (display "name = ")
     (display ((layer 'get-name)))
     (newline)
     (display "streamed=")
     (display ((layer 'get-streamed)))
     (newline)))
 layer-list)