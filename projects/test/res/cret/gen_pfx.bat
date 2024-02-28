:: openssl.exe所在路径
SET openssl_rootpath="C:\Program Files\OpenSSL-Win64\bin"

:: 生成的文件存放路径
SET rootpath="F:\My\ddwork_space\dd\projects\test\res\cret"

:: 生成pfx名称: XXX.pfx
SET name=ddm

:: 生成的pfx密码
SET psw=123456

:: 删除并重新生成 openssl.cfg  
del %rootpath%\openssl.cfg  
(@echo [ req ]
@echo default_bits = 2048
@echo distinguished_name = req_distinguished_name
@echo req_extensions = v3_req
@echo prompt = no

@echo [ req_distinguished_name ]
@echo C = CN
@echo ST = YourState
@echo L = YourCity
@echo O = YourOrganization
@echo OU = YourUnit
@echo CN = localhost

@echo [ v3_req ]
@echo keyUsage = critical, digitalSignature, keyEncipherment
@echo extendedKeyUsage = serverAuth, codeSigning
@echo basicConstraints = critical,CA:FALSE
@echo subjectAltName = @alt_names

@echo [ alt_names ]
@echo DNS.1 = localhost
@echo IP.1 = 127.0.0.1)>%rootpath%\openssl.cfg
:: @echo subjectAltName = @alt_names 是必须的,浏览器会验证

:: 生成私钥
%openssl_rootpath%\openssl genrsa -out %rootpath%\tmp_private.key 2048
:: 生成csr
%openssl_rootpath%\openssl req -new -key %rootpath%\tmp_private.key -out %rootpath%\tmp.csr -config %rootpath%\openssl.cfg
:: 生成crt
%openssl_rootpath%\openssl x509 -req -days 3650 -in %rootpath%\tmp.csr -signkey %rootpath%\tmp_private.key -out %rootpath%\tmp.crt -extensions v3_req -extfile %rootpath%\openssl.cfg
:: 生成自签名pfx
%openssl_rootpath%\openssl pkcs12 -export -out %rootpath%\%name%.pfx -inkey %rootpath%\tmp_private.key -in %rootpath%\tmp.crt -name "%name%" -passout pass:%psw%

:: 删除中间文件
del %rootpath%\tmp_private.key
del %rootpath%\tmp.csr
del %rootpath%\tmp.crt
del %rootpath%\openssl.cfg