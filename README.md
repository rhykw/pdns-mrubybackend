**(8th July 2018) I have decided to discontinue development of pdns-mrubybackend.**

# pdns-mrubybackend [![Build Status](https://travis-ci.org/rhykw/pdns-mrubybackend.svg?branch=master)](https://travis-ci.org/rhykw/pdns-mrubybackend)

## What is pdns-mrubybackend
__pdns-mrubybackend is backend extension module using Scripting Language [mruby](http://www.mruby.org/) for [PowerDNS](https://github.com/PowerDNS/pdns).__

### Notice
Current code of pdns-mrubybackend does not work for pdns-3.x.
If you want to use with pdns-3.x , please use [v1.3.0](https://github.com/rhykw/pdns-mrubybackend/releases/tag/v1.3.0)


## Install

Please use `build.sh`

```sh
sh ./build.sh
sudo cp src/mrubybackend.so /usr/lib64/pdns/
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

2. MaxMindDB(GeoIP) balancing

    using with [mruby-maxminddb](https://github.com/happysiro/mruby-maxminddb).

    pdns-backend.rb
    ```ruby
    $mmdb = MaxMindDB.new "/tmp/GeoLite2-City.mmdb"
    def lookup
      Powerdns::answer.clear
    
      $mmdb.lookup_string "#{Powerdns::Request::remote_addr}"
      cc = $mmdb.country_code
    
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
