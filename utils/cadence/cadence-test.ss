(module cadence-test scheme
  (require "cadence.ss")
  (define input-list (vector->list (current-command-line-arguments)))

  ;(define collected-strings (foldl (lambda (word result) 
  ;                   (append result (readlines word))) '() input-list))
  ;(write-to-file "tell.tll" (append (parse collected-strings) (layer-setup) (post-proceed)))
  ;(debug-print-packets "d:\\toped\\vanguard\\display.drf")
  (convert (list "d:\\toped\\vanguard\\tell.tll" "d:\\toped\\vanguard\\display.drf" "d:\\toped\\vanguard\\vis40cb.tf"))
  ;(convert input-list)
  ;(convert (list "tellx.tll" "default.ss" "techfile.ss"))
  )