put 84
put 101
put 115
put 116
put 58
put 10

read a
read b

let sum + a b
let diff - a b
let prod * a b
let quot / a b
let rem % a b


put 43
write sum
put 10
put 45
write diff
put 10
put 42
write prod
put 10
put 47
write quot
put 10
put 37
write rem
put 10

let eq == a b
let ne != a b
let lt < a b
let gt > a b
let le <= a b
let ge >= a b

put 61
put 61
write eq
put 10
put 33
put 61
write ne
put 10
put 60
write lt
put 10
put 62
write gt
put 10
put 60
put 61
write le
put 10
put 62
put 61
write ge
put 10

let andval & != 0 a != 0 b
let orval | == 0 a == 0 b

put 38
write andval
put 10
put 124
write orval
put 10

if > a b
  put 71
else
  if == a b
    put 69
  else
    put 76
  end
end
put 10

if == 0 % a 2
  put 69
end
put 10

let fact 1
let i a
while > i 0
   let fact * fact i
   let i - i 1
end
put 70
write fact
put 10

if > a 0
   let j 0
   while < j a
      put 35
      let j + j 1
   end
end
put 10

put 71
put 101
put 116
put 58
put 10
get ch
let ascii ch
write ascii
put 10

put 65
put 100
put 100
put 114
put 58
put 10

let @ 123 a
write @ 123
put 10
let @ 456 b
let @ 123 @ 456
write @ 123
put 10

put 73
put 110
put 102
put 105
put 120
put 58
put 10

let x ((a + b) * 2)
let y ((a / 2) - b)
let z (x + y)
write z
put 10

put 68
put 111
put 110
put 101
put 10
end