(module virtuoso2tll scheme
  (require "cadence.ss")
  (define input-list (vector->list (current-command-line-arguments)))

  ;(define collected-strings (foldl (lambda (word result) 
  ;                   (append result (readlines word))) '() input-list))
  ;(write-to-file "tell.tll" (append (parse collected-strings) (layer-setup) (post-proceed)))

  ;(convert (list "default.ss" "techfile.ss"))
  (convert (list "d:\\1\\tell4.tll" "d:\\1\\default.drf" "d:\\1\\techfile.tf"))
  ;(display (car input-list))
  ;(display (cadr input-list))
  ;(display (caddr input-list))
  ;(convert input-list)
  
  ;(convert (list "d:\\1\\tell4.tll" "d:\\1\\default.drf" "d:\\1\\techfile.tf"))
  )