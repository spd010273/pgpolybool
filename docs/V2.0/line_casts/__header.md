# Line Casting

These functions cast various PostgreSQL geometric types to a LINE. Lines are expressed as

```c
typedef struct
{
    double A,
           B,
           C;
} LINE;
```

Where the values A,B,C form the line equation `Ax + By + C = 0`

pgpolybool uses the simpler notation `y = Mc + B` internally, and will often return a LINE with the B variable set to -1.0
