---
published: false
---
I have been trying [Atom](https://atom.io) to see if I can settle with it once and for all coding requirements. It has got nice packages. One such package is `script` which allows one to run all possible programs from the Atom editor itself.

However, there was one issue. It was running python instead of python3, which is what I needed. After googling around a bit, I realized this hack works good. 

Open and look for the following lines:

    .atom/packages/script/lib/grammars/python.js

```
export const Python = {
  "Selection Based": {
    command: "python",
    args(context) {
      setEncoding()
      const code = context.getCode()
      const tmpFile = GrammarUtils.createTempFileWithCode(code)
      return ["-u", tmpFile]
    },
  },

  "File Based": {
    command: "python",
    args({ filepath }) {
      setEncoding()
      return ["-u", filepath]
    },
  },
}
```

Replace `python` with `python3` and you are done!
