set tech=simple.txt

set file1=sref_hier.
set file2=sref_3inst_norot_noflip_nomag.
set file3=sref_3inst_norot_flip_nomag.


call create_vrml.bat %file1%gds %file1%wrl %tech% bbb
call create_vrml.bat %file2%gds %file2%wrl %tech% bbb
call create_vrml.bat %file3%gds %file3%wrl %tech% bbb
