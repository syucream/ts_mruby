discount = Discount.new("http://kevinburke.bitbucket.org/markdowncss/markdown.css", "Markdown Web page example")
md = <<'EOS'
# Markdown Web page example

* You can write web pages by Markdown format.
* The doc will converted to html when HTTP requests come.

## Requirements

* [mruby-discount](https://github.com/matsumoto-r/mruby-discount)

## Example

* This .rb file is it!
EOS

html = discount.header
html << discount.md2html(md)
html << discount.footer

ATS::rputs html
