set tech=simple.txt

set file1=aref_5x10_norot_flip_nomag.
set file2=aref_5x10_norot_noflip_nomag.
set file3=aref_5x10_45_flip_nomag.
set file4=aref_5x10_45_flip_2x.
set file5=aref_5x10_45_flip_0.3x.
set file6=aref_simple.
set file7=aref_5x10_45_noflip_nomag.


call create_vrml.bat %file1%gds %file1%wrl %tech% bbb
call create_vrml.bat %file2%gds %file2%wrl %tech% bbb
call create_vrml.bat %file3%gds %file3%wrl %tech% bbb
call create_vrml.bat %file4%gds %file4%wrl %tech% bbb
call create_vrml.bat %file5%gds %file5%wrl %tech% bbb
call create_vrml.bat %file6%gds %file6%wrl %tech% bbb
call create_vrml.bat %file7%gds %file7%wrl %tech% bbb

