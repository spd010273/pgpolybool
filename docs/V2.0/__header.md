V2.0
====

The following table describes the current status of type interoperability in PostgreSQL with pgpolybool supplied. The table cells list the provider of the cast type.

| From / To | POLYGON | POINT | LSEG | LINE | PATH | BOX | CIRCLE | VECTOR | NORMAL |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| POLYGON | PostgreSQL | PostgreSQL | pgpolybool | pgpolybool | PostgreSQL | PostgreSQL | PostgreSQL | N/A | N/A |
| POINT | pgpolybool | PostgreSQL | pgpolybool | pgpolybool | N/A | PostgreSQL | N/A | N/A | N/A |
| LSEG | pgpolybool | PostgreSQL | pgpolybool | pgpolybool | N/A | N/A | N/A | N/A | N/A |
| LINE | pgpolybool | pgpolybool | pgpolybool | PostgreSQL | N/A | N/A | N/A | N/A | N/A | N/A |
| PATH | PostgreSQL | pgpolybool | pgpolybool | pgpolybool | PostgreSQL | N/A | N/A | N/A | N/A |
| BOX | PostgreSQL | PostgreSQL | PostgreSQL | pgpolybool | N/A | PostgreSQL | PostgreSQL | N/A | N/A |
| CIRCLE | PostgreSQL | PostgreSQL | pgpolybool | pgpolybool | N/A | PostgreSQL | PostgreSQL | N/A | N/A |
| VECTOR | N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A | pgpolybool | N/A |
| NORMAL | N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A | pgpolybool |
