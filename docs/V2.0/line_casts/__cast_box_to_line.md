## __cast_box_to_line

### Prototype:

LINE __cast_box_to_line( BOX )

### Description:

Casts the input BOX to a line by generating the line equation for the diagonal of the BOX. Unfortunately, this line can form either diagonal of the box.

### Error Conditions:

Emits an error and aborts the transaction on failure to allocate memory.
