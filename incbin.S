.section .time_critical.firmware
.global firmware
.type firmware, %object
.balign 4
firmware:
.incbin "../applicard-v9.bin"
