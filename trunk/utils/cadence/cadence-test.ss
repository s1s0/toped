(module cadence-test scheme
  (require "cadence.ss")
  (define input-list (vector->list (current-command-line-arguments)))

  ;(define collected-strings (foldl (lambda (word result) 
  ;                   (append result (readlines word))) '() input-list))
  ;(write-to-file "tell.tll" (append (parse collected-strings) (layer-setup) (post-proceed)))
  ;(debug-print-packets "d:\\toped\\vanguard\\display.drf")
  (convert (list "d:\\toped\\v\\tell.tll" "d:\\toped\\v\\display.drf" "d:\\toped\\v\\vis40cb.tf"))
  ;(convert input-list)
  ;(convert (list "tellx.tll" "default.ss" "techfile.ss"))
  )