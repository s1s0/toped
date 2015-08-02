## Stipple Patterns using Mesa ##

It was reported that under Linux Toped fill patterns doesn't work with ATI graphic cards of [R300](http://en.wikipedia.org/wiki/Radeon_R300) family. The problem was identified to be in the [DRI](http://dri.freedesktop.org/wiki/R300) driver and [reported](http://www.mail-archive.com/dri-devel@lists.sourceforge.net/msg36502.html) to the developers. Unfortunately it is highly unlikely that this driver functionality will be implemented as it was explained [here](http://www.mail-archive.com/dri-devel@lists.sourceforge.net/msg36508.html).

The alternative is to use [ATI driver](http://ati.amd.com/support/drivers/linux/linux-radeon.html).

The list of potentially affected graphic cards can be found [here](http://en.wikipedia.org/wiki/Comparison_of_ATI_Graphics_Processing_Units#Radeon_R300_.289xxx.2C_X10xx.29_series)