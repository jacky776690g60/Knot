# Knot

`Knot` is an encrypter and decrypter for targeted files.

> <h2 id='toc0'>Table of Content</h2>

1. <a href='#build'>build</a>
2. <a href='#Run'>Run</a>
3. <a href='#Requirements'>Requirements</a>
4. <a href='#HowToUse'>How to Use</a>
5. <a href='#SupportedOS'>Supported OS</a>

<h1 id="build" style="font-weight: 700; text-transform: capitalize; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #EA638C;">&#9698; build</h1>
<a href='#toc0' style='background: #000; margin:0 auto; padding: 5px; border-radius: 5px;'>Back to ToC</a><br><br>
- MacOS
  1. Run `./build.sh --force` to build from CMake if this is the first time you're building it; otherwise, `./build.sh`


<h1 id="Run" style="font-weight: 700; text-transform: capitalize; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #EA638C;">&#9698; Run</h1>
<a href='#toc0' style='background: #000; margin:0 auto; padding: 5px; border-radius: 5px;'>Back to ToC</a><br><br>

1. run `./encrypter` or `./decrypter`
2. Enter a password
3. (Optional) run `./cleaner` to remove all .knot files


<h1 id="Requirements" style="font-weight: 700; text-transform: capitalize; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #EA638C;">&#9698; Requirements</h1>
<a href='#toc0' style='background: #000; margin:0 auto; padding: 5px; border-radius: 5px;'>Back to ToC</a><br><br>
1. CMake (3.10+)



<h1 id="HowToUse" style="font-weight: 700; text-transform: capitalize; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #EA638C;">&#9698; How to Use</h1>
<a href='#toc0' style='background: #000; margin:0 auto; padding: 5px; border-radius: 5px;'>Back to ToC</a><br><br>
1. build the executables
2. copy the `knot/` folder inside of `build/dist/` to the root folder of your other project
3. In that project, `cd knot/` and then run `./encrypter` or `./decrypter`
4. Enter a password for either encryption or decryption 



<h1 id="SupportedOS" style="font-weight: 700; text-transform: capitalize; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: #EA638C;">&#9698; Supported OS</h1>
<a href='#toc0' style='background: #000; margin:0 auto; padding: 5px; border-radius: 5px;'>Back to ToC</a><br><br>
- MacOS
- Windows
- Linux