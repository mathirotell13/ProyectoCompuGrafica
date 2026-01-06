#ifndef PERSONA_H
#define PERSONA_H


class Persona
{
    public:
        Persona(const std::string & rutaOBJ);
        //virtual ~Persona();
        std::string getRuta();
        void setRuta(const std::string & ruta);

    //protected:

    private:
        std::string rutaOBJ;
};

#endif // PERSONA_H
