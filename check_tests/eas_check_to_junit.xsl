<?xml version="1.0" encoding="Windows-1252"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
xmlns:chk="http://check.sourceforge.net/ns"
exclude-result-prefixes="chk">



    <xsl:output encoding="utf-8" indent="yes"/>

    <xsl:template match="chk:testsuites">
        <testsuite>
            <xsl:attribute name="errors">
                <xsl:value-of select="count(//chk:test[@result='error'])"/>
            </xsl:attribute>
            <xsl:attribute name="failures">
                <xsl:value-of select="count(//chk:test[@result='failure'])"/>
            </xsl:attribute>
            <xsl:attribute name="tests">
                <xsl:value-of select="count(//chk:test[@result='success'])"/>
            </xsl:attribute>
            <xsl:attribute name="name">
                <xsl:value-of select="//chk:description"/>
            </xsl:attribute>
            <xsl:apply-templates select="chk:suite"/>
        </testsuite>
    </xsl:template>

    <xsl:template match="chk:suite">
        <xsl:for-each select="chk:test">
            <xsl:element name="testcase">
                <xsl:attribute name="classname">
                    <xsl:value-of select="chk:description"/>
                </xsl:attribute>
                <xsl:attribute name="name">
                    <xsl:value-of select="chk:id"/>
                </xsl:attribute>
		    	<xsl:if test="@result='failure'">
					<xsl:element name="failure">
						<xsl:attribute name="message">
						    <xsl:value-of select="chk:message"/>
						</xsl:attribute>
					</xsl:element>
		        </xsl:if>
		    	<xsl:if test="@result='error'">
					<xsl:element name="error">
						<xsl:attribute name="message">
						    <xsl:value-of select="chk:message"/>
						</xsl:attribute>
					</xsl:element>
		        </xsl:if>
            </xsl:element>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>
