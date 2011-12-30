
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


data = {
  'okokko' : 2324
  'lllll' : 9898
}

data2 = {
  'aaaa' : 'eeezezez'
  'lllll' : 9
}

data3 = {
  'aaaa' : 'eezzzzzzzzzzzzez'
}

out = extend data, data2, data3

console.log out