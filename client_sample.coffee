
is_url_argument = ()->
  this.type isnt undefined and this.method isnt undefined

###
Ryan LeFevre - http://forrst.com/posts/Deep_Extend_an_Object_in_CoffeeScript-DWu
###
deepExtend = (object, extenders...) ->
  return {} if not object?
  for other in extenders
    for own key, val of other
      if not object[key]? or typeof val isnt "object"
        object[key] = val
      else
        object[key] = deepExtend object[key], val

  object

HTMLSelectElement::restSelector = ( restWSDL )->

  select = this

  this.onchange = ()->
    id = select.selectedIndex
    val = select.options[id].text
    console.log val

  selectdata = '':'---'
  selectdata = deepExtend selectdata, restWSDL

  for option, data of selectdata
    opt = document.createElement 'option'
    opt.setAttribute 'value', option
    opt.setAttribute 'label', option
    opt.textContent = option
    console.log option
    console.log data
    select.appendChild opt

thesel = document.createElement 'select'

thesel.restSelector {"comment":{"create":{"comment":{"type":"array","method":"POST|comment"}},"update":{"cid":{"type":"int","method":"GET|path|0"},"data":{"type":"array","method":"POST|data"}}},"user":{"login":{"username":{"type":"string","method":"POST|username"},"password":{"type":"string","method":"POST|password"}}},"views":{"view_name":{"type":"string","method":"GET|path|0"},"display_id":{"type":"string","method":"GET|param|display_id"},"args":{"type":"array","method":"GET|param|args"},"offset":{"type":"int","method":"GET|param|offset"},"limit":{"type":"int","method":"GET|param|limit"},"theme_output":{"type":"bool","method":"GET|param|theme_output"}}}

wrap = document.getElementById 'repl_source_wrap'
wrap.appendChild thesel
