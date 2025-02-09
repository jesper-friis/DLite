try:
    import requests
except ModuleNotFoundError:
    print("requests not installed, skipping test")
    raise SystemExit(44)  # skip test

import dlite


url = "https://raw.githubusercontent.com/SINTEF/dlite/master/storages/python/tests-python/input/test_meta.json"

meta = dlite.Instance.from_location("http", url)

assert str(meta) == """
{
  "uri": "http://onto-ns.com/meta/0.1/TestEntity",
  "description": "test entity with explicit meta",
  "dimensions": {
    "L": "first dim",
    "M": "second dim",
    "N": "third dim"
  },
  "properties": {
    "myblob": {
      "type": "blob3"
    },
    "mydouble": {
      "type": "float64",
      "unit": "m",
      "description": "A double following a single character..."
    },
    "myfixstring": {
      "type": "string3",
      "description": "A fix string."
    },
    "mystring": {
      "type": "string",
      "description": "A string pointer."
    },
    "myshort": {
      "type": "uint16",
      "description": "An unsigned short integer."
    },
    "myarray": {
      "type": "int32",
      "shape": ["L", "M", "N"],
      "description": "An array string pointer."
    }
  }
}
""".strip()
