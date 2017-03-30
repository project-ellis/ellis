# API design sketches

This document is used for ideas and work in progress.

As the details take shape and become more concrete, they should be moved into
more permanent formal resting places, such as header files or other markdown
files.

## Metadata

When representing large amounts of data, it may be useful to separate out the
metadata describing the records, in order to reduce unnecessary repetition.

Possibly a "sticky metadata" approach might be used, where the metadata for
a given type of field is fixed until something changes it.

This is TBD.

```
{
  "units": "kg/sec",
  "srcInfo": {},
  "algorithm": "blah",
  "version": "0.0.7
}
```

## Schema

Design goal: simplicity of implementation, use, and comprehension, while still
expressive enough for most checks.

We allow derived types, but a derived type has a single derivation path with
no branching that leads back to a base type.  Derived maps can add further
constraints on existing keys.

IDEA: have an optional conversion (undefined by default) that allows fixing a
type mismatch on a sub-element, by way of a conversion function.  Note: the
conversion function must run immediately, and may fail, resulting in schema
match failure.

Base types of EllisNode
* array
* binary
* bool
* double
* int64
* map
* nil
* string

### Example of schema definition:
```json
{
    "ext.positiveInt64": {
        "type": "int64",
        "constraint": [ "gt", 0 ]
    },

    "ext.percentage": {
        "type": "double",
        "constraint": [ "and",
            [ "ge", 0.0 ],
            [ "le", 100.0 ]
        ]
    },


    "ext.nonEmptyString": {
        "type": "string",
        "constraint": [ "gt", ["len"], 0 ]
    },


    "ext.bin128bit": {
        "type": "binary",
        "constraint": [ "eq", ["len"], 16 ]
        "fixup" : {
          "TODO" : "totally not thought out, see optional conversion above"
          "ext.base64str" : "base64decode"
        }
    },


    "example.funkyString": {
        "type": "string",
        "constraint": [
            "and",
            [ "ge", [ "len" ], 4 ],
            [ "if",
                [ "gt", [ "len" ], 10 ],
                [ "regex", "^[a-z0-9]*$" ],
                [ "regex", "^[A-Z0-9]*$" ]
            ],
            [ "regex", "^5" ],
            [ "constraintof", [ "substr", 0, 2 ], "ext.percentage" ]
        ]
    },


    "example.abstractHeaterSetting": {
        "type": "ext.positiveInt64",
        "abstract": "true"
    },


    "example.frontHeaterSetting": {
        "type": "example.abstractHeaterSetting",
        "constraint": [ "lt", 5 ]
    },


    "example.rearHeaterSetting": {
        "type": "example.abstractHeaterSetting",
        "constraint": [ "lt", 3 ]
    },


    "srio.data.xypoint": {
        "type": "map",
        "required": {
                    "x": { "type": "double" },
                    "y": { "type": "double" }
        }
    },


    "srio.data.landmarkTrace": {
        "type": "array",
        "constraint": [ "and", [ "gt", ["len"], 3 ], ["le", ["len"], 50] ],
        "members": [ { "type": "srio.data.xypoint" } ]
    },


    "srio.data.rec.abstract": {
        "type": "map",
        "abstract": "true"
    },


    "srio.data.rec.mysterious": {
        "type": "srio.data.rec.abstract"
    },


    "srio.data.rec.xyz": {
        "type": "srio.data.rec.abstract",
        "required": {
            "x": { "type": "double" },
            "y": { "type": "double" },
            "z": { "type": "double" }
        }
    },


    "srio.data.rec.accelerometer": {
        "type": "srio.data.rec.xyz"
    },


    "srio.data.rec.magnemometer": {
        "type": "srio.data.rec.xyz"
    },


    "srio.data.rec.humidity": {
        "type": "srio.data.rec.abstract",
        "required": {
            "percent": { "type": "double" }
        }
    },


    "srio.data.doc": {
        "type": "map",
        "required": {
            "start": { "type": "int64" },
            "streamId": { "type": "string" },
            "type": { "type": "string" },
            "data": { "type": "srio.data.rec.abstract" },
            "mdHash": { "type": "ext.bin128bit" }
        },
        "optional": {
            "end": { "type": "int64" }
        }
    },


    "elrpc.msgid": {
        "type": "string",
        "constraint": [ "gt", ["len"], 0]
    },


    "elrpc.mode": {
        "type": "string",
        "constraint": [ "regex", "request|response|notify" ]
    },


    "elrpc.msg.abstract": {
        "type": "map",
        "required": {
            "msgId": { "type": "elrpc.msgid" },
            "mode": { "type": "elrpc.mode" },
            "method": { "type": "string" }
        },
        "abstract": true
    },


    "elrpc.request.abstract": {
        "type": "elrpc.msg.abstract",
        "required": {
                    "mode": { "constraint" : [ "eq", "request" ] }
        },
        "abstract": true
    },


    "elrpc.response.abstract": {
        "type": "elrpc.msg.abstract",
        "required": {
            "mode": { "constraint" : [ "eq", "response" ] },
            "status": {
                "type": "map",
                    "required": { "code": { "type": "int64" } }
            }
        },
        "abstract": true
    },


    "srio.dax.req.docdata.write": {
        "type": "elrpc.request.abstract",
        "required": {
            "method": { "constraint" : [ "eq", "srio.dax.method.docdata.write" ] },
            "params": {
                "type": "map",
                "required": {
                    "tableName": { "type": "string" },
                    "documents": {
                        "type": "array",
                        "members": [ { "type": "srio.data.doc" } ],
                        "constraint": [ "gt", ["len"], 0 ]
                    }
                }
            }
        }
    },

    "srio.dax.resp.docdata.write": {
        "type": "elrpc.response.abstract",
        "required": {
            "method": { "constraint" : [ "eq", "srio.dax.method.docdata.write" ] }
        }
    }
```

## Ellispipe

Example of an ellispipe command:

```
ellispipe sel '[:100]' ! sort ! transpose
```
