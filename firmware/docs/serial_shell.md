# Serial text shell (design)

Human-friendly commands over serial, translated into the existing JSON
commands. Handlers and TCP stay unchanged; lines starting with `{` are still
treated as raw JSON. Rendered PNGs: `serial_shell-N.png` (regenerate with
`mmdc -i serial_shell.md -o serial_shell.png -b white -s 2`).

## 1. Current command flow

```mermaid
flowchart LR
    S[SerialTransport] --> CS[CommandSession<br/>readLine loop]
    B[BleTransport] --> CS2[CommandSession]
    T[TcpListener] --> D
    CS --> D[CommandDispatcher<br/>deserializeJson + 'request' lookup]
    CS2 --> D
    D --> H["Handlers<br/>handle(const JsonDocument&, const Context&)"]
    H -- "Reply (doc or error)" --> D
    D -- "transport.send(doc)" --> S
```

## 2. Proposed text front-end

```mermaid
flowchart LR
    U([User in serial monitor]) --> LE[Line editor<br/>echo, backspace, history]
    LE --> P{"starts with #123; ?"}
    P -- yes --> D[CommandDispatcher]
    P -- no --> TP[TextParser<br/>tokens → JsonDocument]
    TP --> D
    D --> H[Handlers - unchanged]
    H -- JSON --> F[TextFormatter<br/>JSON → pretty text]
    F --> U
```

## 3. One `set` command, end to end

```mermaid
sequenceDiagram
    actor User
    participant Shell as Serial shell
    participant Parser as TextParser
    participant Disp as CommandDispatcher
    participant H as SetFixtureHandler
    participant Fmt as TextFormatter

    User->>Shell: set 0 universe=8 name="Front"
    Shell->>Parser: tokens [set, 0, universe=8, name=Front]
    Parser->>Disp: {"request":"setfixture","id":0,"universe":8,"name":"Front"}
    Disp->>H: handle(doc, ctx)
    H-->>Disp: Reply {"status":"ok","id":0,"universe":8,...}
    Disp->>Fmt: send(reply)
    Fmt->>User: ok / id 0 / universe 8 / name Front
```

## 4. Command table

```mermaid
classDiagram
    class TextCommand {
        +string_view verb
        +string_view request
        +string_view positional[]
        +string_view help
        +Formatter* formatter
    }
    class TextParser {
        +parse(line) JsonDocument
    }
    class TextFormatter {
        +generic(JsonVariant) string
    }
    TextParser --> TextCommand : looks up verb
    TextFormatter <-- TextCommand : optional override
```

## 5. Where it plugs into the current stack

`SerialShell` replaces `m_serialSession` (`Core/Engine.h`). The shell picks the
reply format by choosing which transport it passes to the dispatcher.

```mermaid
---
config:
  layout: elk
---
flowchart TD
    subgraph IN [Serial input]
        UART[(UART)] --> LE[LineEditor]
        LOG[Logger / Telemetry] -. redraw prompt .-> LE
        LE --> R{"line starts with #123; ?"}
        R -- no --> TP[TextParser + command table]
        TP -- local cmd --> LOC[help, confirm, did-you-mean]
    end
    subgraph CORE [Unchanged]
        D[CommandDispatcher] --> H[Handlers]
    end
    subgraph OUT [Serial output]
        OUTQ{"which transport<br/>was passed in?"}
        OUTQ -- text line --> TR["TextReply decorator<br/>JSON → text"]
        OUTQ -- raw line --> ST[SerialTransport]
        TR --> ST
    end
    R -- "yes: raw JSON" --> D
    TP -- "JsonDocument" --> D
    H -- send JSON --> OUTQ
```
