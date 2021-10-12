def main():
   riley_score = 0
   kristine_score = 0

   cur_key = "-"

   # press q to quit
   while cur_key != "q":
      if cur_key == "r":
         riley_score += 1
      elif cur_key == "k":
         kristine_score += 1
      else:
         print("Enter a different letter")

      print("Riley: {}".format(riley_score))
      print("Kristine: {}".format(kristine_score))

      cur_key = input()

main()