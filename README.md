# pdns-mrubybackend

## What is pdns-mrubybackend
__pdns-mrubybackend is backend extension module using Scripting Language [mruby](http://www.mruby.org/) for [PowerDNS](https://github.com/PowerDNS/pdns).__


## Install

Please use `build.sh`

```
sh ./build.sh
```

## Examples

1. rand response

    pdns-backend.rb
    ```ruby
    def lookup
      Powerdns::answer.clear

      records = [
        {"name"=>"example.com",     "type"=>"SOA" ,"content"=>"ns.example.com. hostmaster.example.com. 1 1800 900 604800 3600",},
        {"name"=>"rand.example.com","type"=>"TXT" ,"content"=>rand.to_s,},
      ]

      records.each{|rec|
        if( rec["name"] == Powerdns::Request::domain && (Powerdns::Request::type == "ANY" || rec["type"] == Powerdns::Request::type) )
          Powerdns::answer.push( rec )
        end
      }
    end
    ```
    response
    ```sh
    $ dig @172.17.42.53 ANY rand.example.com.

    ;; QUESTION SECTION:
    ;rand.example.com.      IN  ANY

    ;; ANSWER SECTION:
    rand.example.com.   120 IN  TXT "0.2835277512864"
    ```

2. geoip balancing

    pdns-backend.rb
    ```ruby
    $db_path = "/usr/share/GeoIP/GeoIPCity.dat"
    $geoip   = GeoIP.new $db_path
    def lookup
      Powerdns::answer.clear
    
      $geoip.record_by_addr "#{Powerdns::Request::remote_addr}"
      cc = $geoip.country_code
    
      records = [
        {"name"=>"example.com"     ,"type"=>"SOA" ,"content"=>"ns.example.com. hostmaster.example.com. 1 1800 900 604800 3600",},
        {"name"=>"geo.example.com" ,"type"=>"TXT" ,"content"=>"COMMENT: Your country is "+cc,},
        {"name"=>"geo.example.com" ,"type"=>"A"   ,"content"=>({"JP"=>"172.17.43.80","US"=>"172.17.44.80","TW"=>"172.17.45.80"}[cc]||"172.17.46.80")},
      ]
    
      records.each{|rec|
        if( rec["name"] == Powerdns::Request::domain && (Powerdns::Request::type == "ANY" || rec["type"] == Powerdns::Request::type) )
          Powerdns::answer.push( rec )
        end
      }
    end
    ```
    response
    ```sh
    dig @172.17.42.53 ANY geo.example.com.

    ;; QUESTION SECTION:
    ;geo.example.com.       IN  ANY

    ;; ANSWER SECTION:
    geo.example.com.    120 IN  TXT "COMMENT: Your country is US"
    geo.example.com.    120 IN  A   172.17.44.80
    ```

# License
under the MIT License:

* http://www.opensource.org/licenses/mit-license.php
