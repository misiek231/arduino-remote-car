"use strict";
foo();
async function foo() {
    let a = await fetch("/hello");
    var text = document.querySelector(".hello");
    if (text)
        text.innerHTML = await a.text();
}
