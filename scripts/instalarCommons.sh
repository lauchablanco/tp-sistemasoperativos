COMMONS=/home/utnso/so-commons-library

if [ -d "${COMMONS}" ]; then
	echo "Existen commons"
else
	cd /home/utnso/
	git clone https://github.com/sisoputnfrba/so-commons-library.git
	cd ${COMMONS}
	sudo make install
fi

