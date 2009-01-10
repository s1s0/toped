(module cadence-test scheme
  (require "cadence.ss")
  (define input-list (vector->list (current-command-line-arguments)))

  ;(define collected-strings (foldl (lambda (word result) 
  ;                   (append result (readlines word))) '() input-list))
  ;(write-to-file "tell.tll" (append (parse collected-strings) (layer-setup) (post-proceed)))

  ;(convert (list "default.ss" "techfile.ss"))
  (convert input-list)
  ;(convert (list "tellx.tll" "default.ss" "techfile.ss"))
  )