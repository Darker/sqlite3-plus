// this script was used to generate error code enum from sqlite3 docs

(()=>{
  const NL = "\n";
  const REGEX_CODE = /\s*\([[0-9]+\)/;
  REGEX_CODE.test("(1) SQLITE_ERROR");
  
  let resView = document.querySelector("#resview");
  if(!resView) {
    resView = document.createElement("pre");
    resView.id = "resview";
    resView.addEventListener("click", async()=>{
      const res = await navigator.clipboard.writeText(window.generatedCPPCode);
      console.log(res);
      
    });
    //resView.stype.whitespace = "pre";
    document.body.insertBefore(resView, document.body.firstChild);
  }
  resView.innerHTML = "";
  
  const elms = [...document.querySelectorAll("h3")]
         .filter(x => REGEX_CODE.test(x.textContent));
  let cppCode = "#pragma once" + NL+ NL
                +"namespace sqlitepp" + NL
                +"{" + NL+ NL
                +"enum class ResultCode"+NL+"{"+NL;
  for(const elm of elms) {
    // create comment from description
    let descEl = elm.nextElementSibling;
    let first = 0;
    while(descEl && descEl.tagName.toLowerCase() != "h3") {
      if(first++ == 0) {
        cppCode += "  /**" + NL;
      }
      else {
        cppCode += NL;
      }
      cppCode += StringifyNode(descEl, 0);
      //console.log(descEl);
      descEl = descEl.nextElementSibling;
    }
    
    if(first > 0)
      cppCode += "   **/"+NL;
    const match = /\(([0-9]+)\)\s*SQLITE_([A-Z_]+)/.exec(elm.textContent);
    cppCode += "  " + match[2] + " = " + " ".repeat(25-match[2].length) + match[1] + ","+NL;
    
  }
  cppCode += "};"+NL+"}" +NL;
  resView.appendChild(new Text(cppCode));
  //console.log(cppCode);
  window.generatedCPPCode = cppCode;
})();

function StringifyNode(elm, padding = 0) {
  let res = "";
  const padstr = "   * " + " ".repeat(padding);
  if(elm.tagName.toLowerCase() == "p") {
    res += padstr + FormatTextContent(elm.textContent, padstr) + NL;
  }
  else if(elm.tagName.toLowerCase() == "ol") {
    let index = 0;
    for(const el of elm.querySelectorAll("li")) {
      res += padstr + " " + (++index) + ". " + FormatTextContent(elm.textContent, padstr + "    ") + NL;
    }
  }

  return res;
}

function FormatTextContent(text, padstr) {
  text = text.replace(/[\r\n\t]/g, " ").replace(/ {2,}/g, " ").replace(/^\s+/g, "").split(" ");
  let res = "";
  let chars = 0;
  for(const word of text) {
    
    if(chars > 80) {
      chars = 0;
      res += "\r\n" + padstr;
    }
    if(chars != 0) {
      res += " ";
      chars++;
    }
    chars += word.length;
    res += word;
  }
  return res;
}