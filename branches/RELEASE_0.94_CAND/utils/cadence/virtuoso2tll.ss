#! /usr/local/bin/mzscheme
(module virtuoso2tll scheme
  (require "cadence.ss" )
  (define input-list (vector->list (current-command-line-arguments)))
  (cond ((or (empty? input-list)
             (< (length input-list) 3)) (begin
                               (display "Not enough input parameters")
                               (newline)
                               (display "Usage: virtuoso2tll outfile.tll display1.drf <display2.drf ...> techfile")
                               (newline)))
        (else (convert input-list)))
  
  (read-char)
  )