from pathlib import Path

Import("env")

source = Path(env.subst("$PROJECT_DIR")) / "web" / "control.html"
build = Path(env.subst("$BUILD_DIR"))
build.mkdir(parents=True, exist_ok=True)
output = build / "control_page.h"
html = source.read_text(encoding="utf-8")
if ")CONTROLHTML\"" in html:
    raise ValueError("HTML contains the C++ raw string delimiter")
content = "#pragma once\n#include <Arduino.h>\nconst char CONTROL_PAGE[] PROGMEM = R\"CONTROLHTML(" + html + ")CONTROLHTML\";\n"
if not output.exists() or output.read_text(encoding="utf-8") != content:
    output.write_text(content, encoding="utf-8")
env.Append(CPPPATH=[str(build)])
