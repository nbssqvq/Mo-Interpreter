read year

if == 0 % year 400
   put 76
else
   if != 0 % year 4
     put 67
   else
     if == 0 % year 100
        put 67
     else
        put 76
     end
   end
end

end